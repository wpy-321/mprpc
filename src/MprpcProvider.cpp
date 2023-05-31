#include "MprpcProvider.h"
#include "MprpcApplication.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"
using namespace std;

void MprpcProvider::NotifyService(google::protobuf::Service *service){
    ServiceInfo info;
    info.m_service = service;//保存服务对象

    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* p_serviceDescriptor = service->GetDescriptor();

    //获取服务对象名字
    string serviceName = p_serviceDescriptor->name();
    cout << "serviceName:" << serviceName << endl;

    //获取服务对象包含的方法数量
    int methodCount = p_serviceDescriptor->method_count();

    for(int i = 0;i<methodCount;i++){
        //获取了服务对象指定下标的服务方法的描述（抽象描述） 如UserService第一个服务方法为Login
        const google::protobuf::MethodDescriptor* p_methodDescriptor = p_serviceDescriptor->method(i);
        //获取方法名
        string methodName = p_methodDescriptor->name();
        cout << "methodName:" << methodName << endl;
        info.m_methodMap.insert({methodName, p_methodDescriptor});//保存方法
    }
    m_serviceMap.insert({serviceName,info});//保存服务对象
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void MprpcProvider::run(){
    string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);
    //创建Tcpserver对象
    muduo::net::TcpServer server(&m_eventLoop,address,"mpRpcProvider");

    //绑定连接回调和消息读写回调方法  分离了网络代码和业务代码
    server.setConnectionCallback(bind(&MprpcProvider::OnConnection,this,placeholders::_1));
    server.setMessageCallback(bind(&MprpcProvider::OnMessage,this,placeholders::_1,
                        placeholders::_2,placeholders::_3));

    server.setThreadNum(4);

    /*
    * 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    * session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    */
    ZkClient zk;
    zk.start();
    for(auto &service:m_serviceMap){
        string service_path = "/" + service.first;
        zk.create(service_path.c_str(),nullptr,0);

        for(auto &method:service.second.m_methodMap){
            string method_path = service_path + "/" + method.first;

            char method_value[128];
            memset(method_value,0,sizeof(method_value));
            sprintf(method_value,"%s:%d",ip.c_str(),port);

            // ZOO_EPHEMERAL表示znode是一个临时性节点
            // ZOO_SEQUENCE 表示znode是一个永久性节点
            zk.create(method_path.c_str(),method_value,strlen(method_value),ZOO_EPHEMERAL);
        }
    }


    // rpc服务端准备启动，打印信息
    cout << "RpcProvider start service at ip:" << ip << " port:" << port << endl;
    server.start();
    m_eventLoop.loop();//相当于epoll_wait()
}

//新的socket连接回调
void MprpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn){
    if(!conn->connected()){
        conn->shutdown();
    }
}
/*
* 在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
* service_name method_name args_str: header_size(4个字节) + header_str(包含args_str的长度) + args_str
* 防止TCP粘包问题，所以传输时也传输参数的长度args_size
* 定义proto的message类型，进行数据头的序列化和反序列化:service_name method_name args_size
* 
*/
// 已建立连接用户的读写事件回调
void MprpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buffer,
                            muduo::Timestamp)
{
    // 网络上接收的远程rpc调用请求的字符流
    string recf_buf = buffer->retrieveAllAsString();
    //从字符流中读取前4个字节的内容 也就是header_size
    uint32_t header_size;
    recf_buf.copy((char*)&header_size,4,0);

    // 根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    string header_str = recf_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    string service_name;
    string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(header_str)){
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }else{
        cout << "header_str:" << header_str << " parse error!" << endl;
    }
    
    // 获取rpc方法参数的字符流数据
    string args_str = recf_buf.substr(4+header_size,args_size);

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;
    
    //根据反序列化得到的服务对象名查看是否有这个服务对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end()){
        cout << service_name << " is not exist!" << endl;
        return ;
    }

    //根据反序列化得到的调用方法名查看是否有这个调用方法
    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end()){
        cout << service_name << ":" << method_name << " is not exist!" << endl;
        return;
    }

    google::protobuf::Service* service = it->second.m_service;//获取服务对象 例如UserService
    const google::protobuf::MethodDescriptor* method = mit->second; //获取服务对象的方法 例如login

    // 从服务对象中获取rpc方法调用的请求request和响应response参数
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        cout << "request parse error, content:" << args_str << endl;
        return;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<MprpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>(
        this,&MprpcProvider::SendRpcResponse,conn,response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    service->CallMethod(method,nullptr,request,response,done);
}
// Closure的回调操作，用于序列化rpc的响应和网络发送
void MprpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message *response){
    string response_str;
    if(response->SerializeToString(&response_str)){
        conn->send(response_str);
    }else{
        cout << "serialize response_str error!" << endl;
    }
    // 模拟http的短链接服务，成功调用后就断开连接
    conn->shutdown();
}