#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <google/protobuf/descriptor.h>
#include "google/protobuf/service.h"
using namespace std;
class MprpcProvider{
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);
    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void run();
private:
    muduo::net::EventLoop m_eventLoop;

    //服务对象信息
    struct ServiceInfo{
        google::protobuf::Service *m_service;// 服务对象
        //服务对象中的服务方法 方法名：方法
        unordered_map<string,const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    // 存储注册成功的服务对象和其服务方法的所有信息 服务对象名：{服务对象，方法}
    unordered_map<string,ServiceInfo> m_serviceMap;

    //新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    // 已建立连接用户的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*,muduo::Timestamp);

    void SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message *response);
};