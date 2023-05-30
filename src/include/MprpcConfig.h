#pragma once
#include <string>
#include <unordered_map>
using namespace std;

class MprpcConfig{
public:
    void LoadConfigFile(const string& fileName);

    string Load(const string& key);
private:
    unordered_map<string,string> m_configMap;
    //去除字符串前后的空格
    void Trim(string &str);
};