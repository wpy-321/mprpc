#pragma once
#include "MprpcConfig.h"
#include "MprpcChannel.h"
#include "MprpcController.h"

class MprpcApplication
{
public:
    static void Init(int argc,char** argv);
    //单例模式 返回一个实例
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;

    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};

