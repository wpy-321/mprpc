#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h>
#include <memory>
#include "MprpcApplication.h"
#include "MprpcChannel.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"
using namespace std;
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    /*
     * 1. 先生成格式化的调用信息
     * header_size + service_name method_name args_size + args
     */
    // 通过method获取rpc方法的服务对象
    const google::protobuf::ServiceDescriptor *service = method->service();
    string serviceName = service->name();
    string methodName = method->name();

    uint32_t args_size;
    string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("request->SerializeToString() error");
        return;
    }

    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(serviceName);
    rpcHeader.set_method_name(methodName);
    rpcHeader.set_args_size(args_size);
    uint32_t header_size = 0;
    std::string header_str;
    if (rpcHeader.SerializeToString(&header_str))
    {
        header_size = header_str.size();
    }
    else
    {
        controller->SetFailed("rpcHeader.SerializeToString() error");
        return;
    }

    string send_str;
    send_str.insert(0, string((char *)&header_size, 4));
    send_str += header_str;
    send_str += args_str;

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << header_str << std::endl;
    std::cout << "service_name: " << serviceName << std::endl;
    std::cout << "method_name: " << methodName << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "============================================" << std::endl;

    /*
     * 2. 使用tcp编程，完成rpc方法的远程调用
     */
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errmsg[512];
        memset(errmsg,0,512);
        sprintf(errmsg,"-1 == clientfd,errno:%d",errno);
        controller->SetFailed(errmsg);
        return;
    }
    shared_ptr<int> sp(new int, [](int *clientfd) {close(*clientfd);});
    sp = make_shared<int>(clientfd);

    // // 读取配置文件rpcserver的信息
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    /*rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息*/
    ZkClient zk;
    zk.start();
    string method_path = "/" + serviceName + "/" + methodName;

    string ip_port = zk.getData(method_path.c_str());
    if(ip_port == ""){
        controller->SetFailed(method_path + " is not exist!");
        return ;
    }
    int index = ip_port.find(":");
    if(index == -1){
        controller->SetFailed(method_path + " address is invalid!");
        return ;
    }
    string ip = ip_port.substr(0,index);
    uint16_t port = atoi(ip_port.substr(index+1).c_str());


    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);

    //与rpc节点发起TCP连接
    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        char errmsg[512];
        memset(errmsg,0,512);
        sprintf(errmsg,"connect error,errno:%d",errno);
        controller->SetFailed(errmsg);
        return ;
    }
    //发送rpc请求
    if(-1 == send(clientfd,send_str.c_str(),send_str.size(),0)){
        char errmsg[512];
        memset(errmsg,0,512);
        sprintf(errmsg,"send error,errno:%d",errno);
        controller->SetFailed(errmsg);
        return ;
    }
    //接收rpc请求的响应值
    char recv_buf[1024];
    memset(recv_buf,0,1024);
    int recv_size = 0;
    if(-1 == (recv_size = recv(clientfd,recv_buf,1024,0))){
        char errmsg[512];
        memset(errmsg,0,512);
        sprintf(errmsg,"recv error,errno:%d",errno);
        controller->SetFailed(errmsg);
        return ;
    }

    //反序列化接收到的rpc请求的响应值
    if(!response->ParseFromArray(recv_buf,recv_size)){
        char errmsg[512];
        memset(errmsg,0,512);
        sprintf(errmsg,"parse error,errno:%d",errno);
        controller->SetFailed(errmsg);
        return ;
    }
}