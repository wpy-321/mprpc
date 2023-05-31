#include "user.pb.h"
#include "MprpcApplication.h"

int main(int argc, char **argv)
{
    // 使用mprpc框架来享受rpc服务调用，一定需要先初始化框架
    MprpcApplication::Init(argc, argv);

    /*调用rpc方法*/
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel);

    // 把rpc方法的参数准备好
    fixbug::LoginRequest request;
    request.set_name("wpy");
    request.set_pwd("123456");

    // 准备接收rpc方法的响应
    fixbug::LoginResponse response;

    MprpcController controller;
    // 发起rpc方法的调用  同步的rpc调用过程(会阻塞等待)
    stub.Login(&controller, &request, &response, nullptr);

    if (controller.Failed())
    {
        cout << "rpc login call failed" << controller.ErrorText() << endl;
    }
    else
    {
        // 一次rpc调用完成，读调用的结果
        if (0 == response.result().errcode())
        {
            cout << "rpc login response success:" << response.success() << endl;
        }
        else
        {
            cout << "rpc login response error : " << response.result().errmsg() << endl;
        }
    }

    return 0;
}