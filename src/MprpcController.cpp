#include "MprpcController.h"

MprpcController::MprpcController(){
    m_failed = false;
    m_errmsg = "";
}
//重置控制器
void MprpcController::Reset(){
    m_failed = false;
    m_errmsg = "";
}
//返回状态 成功false 失败true
bool MprpcController::Failed() const{
    return m_failed;
}
//返回错误信息
string MprpcController::ErrorText() const{
    return m_errmsg;
}
//当发生错误的时候 设置错误
void MprpcController::SetFailed(const std::string& reason){
    m_failed = true;
    m_errmsg = reason;
}
// 目前未用的功能
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled() const {return false;}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}