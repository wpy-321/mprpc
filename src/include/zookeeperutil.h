#pragma once
#include <string>
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include "MprpcApplication.h"
using namespace std;
class ZkClient{
public:
    ZkClient();
    ~ZkClient();

    //客户端发起对zkserver的连接
    void start();
    //在zkserver根据给定的路径创建节点
    void create(const char* path,const char* data,int datalen,int state = 0);
    //根据指定的路径节点获取节点的值
    string getData(const char* path);

private:
    //客户端句柄
    zhandle_t* m_zkhandle;
};