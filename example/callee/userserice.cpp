#include <iostream>
#include <string>
#include "user.pb.h"
#include "MprpcApplication.h"
#include "RpcProvider.h"
using namespace std;
using namespace fixbug;

// UserService原本是一个本地服务，现在通过几个操作注册到rpc节点上
class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(string &name, string &pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        cout << name << " " << pwd << endl;
        return true;
    }
    //重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    //caller 序列化Login(LoginRequest),通过muduo网络发送给callee
    //callee 反序列化得到调用的函数，参数，需要返回的值
    //callee只需要直接使用request反序列化得到的参数 然后调用本地方法，把响应交给rpc框架序列化
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        string name = request->name();
        string pwd = request->pwd();
        // 做本地业务
        bool myresult = Login(name,pwd);
         // 把响应写入框架中  包括错误码、错误消息、返回值
        ResultCode* rc =  response->mutable_result();
        rc->set_errcode(1);
        rc->set_errmsg("error!");
        response->set_success(myresult);
        // 执行回调操作，就是告诉框架执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
};

int main(int argc,char** argv)
{
    // 调用框架的初始化操作
    MprpcApplication::Init(argc,argv);

    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上,负责序列化和反序列化以及网络的收发
    RpcProvider provider;
    provider.NotifyService(new UserService);

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.run();
}