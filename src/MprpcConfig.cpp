#include <iostream>
#include <fstream>
#include "MprpcConfig.h"
using namespace std;

void MprpcConfig::Trim(string &str){
    int index = str.find_first_not_of(' ');
    if(index != -1){
        //前面有空格
        str = str.substr(index,str.size()-index);
    }
    index = str.find_last_not_of(' ');
    if(index != -1){
        //前面有空格
        str = str.substr(0,index+1);
    }
}

void MprpcConfig::LoadConfigFile(const string& fileName){
    if(fileName.empty()){
        cout << "文件名为空！" << endl;
    }

    ifstream fin(fileName);
    string line;
    while(getline(fin,line)){
        if(line.find('#') != -1) continue;

        Trim(line);

        int index = line.find('=');
        if(index == -1){
            continue;
        }

        string key;
        string value;
        key = line.substr(0,index);
        Trim(key);
        value = line.substr(index+1);
        Trim(value);
        
        m_configMap.insert({key,value});
    }
}

string MprpcConfig::Load(const string& key){
    if(m_configMap.find(key) != m_configMap.end()){
        return m_configMap[key];
    }

    return "";
}
