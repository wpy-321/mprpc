#include <iostream>
#include <string>
#include <unistd.h>
#include "MprpcApplication.h"
using namespace std;

//静态成员必须类外定义
MprpcConfig MprpcApplication::m_config;
void MprpcApplication::Init(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "请按格式输入配置文件！" << endl;
        exit(EXIT_FAILURE);
    }

    //解析命令行参数，并把配置文件名称读入到config_file中
    int opt = 0;
    string config_file;
    while((opt = getopt(argc,argv,"i:")) != -1){
        switch(opt){
            case 'i':
                config_file = optarg;

                break;
            case '?':
                cout << "?" << endl;
                exit(EXIT_FAILURE);
                break;
            case ':':
                cout << ":" << endl;
                exit(EXIT_FAILURE);
                break;
            default:
                break;
        }
    }
    
    //解析配置文件
    m_config.LoadConfigFile(config_file);
    cout<< "读取配置成功！" << endl;
}

MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication obj;
    return obj;
}
MprpcConfig& MprpcApplication::GetConfig(){
    return m_config;
}