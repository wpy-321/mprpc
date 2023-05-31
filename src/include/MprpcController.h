#pragma once
#include <string>
#include <google/protobuf/service.h>
using namespace std;

class MprpcController:public google::protobuf::RpcController{
public:
    MprpcController();
    void Reset();
    bool Failed() const;
    string ErrorText() const;
    void SetFailed(const std::string& reason);

    // 目前未用的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed;
    string m_errmsg;

};