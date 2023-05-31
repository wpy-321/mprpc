#include "zookeeperutil.h"

using namespace std;

ZkClient::ZkClient():m_zkhandle(nullptr){

}

ZkClient::~ZkClient(){
    if(m_zkhandle != nullptr){
        zookeeper_close(m_zkhandle);//关闭连接
    }
}
/*
* zookeeper_mt：多线程版本
* zookeeper的API客户端程序提供了三个线程
* 1.API调用线程 
* 2.网络I/O线程  pthread_create  poll
* 3.watcher回调线程 pthread_create
*/  
void my_watcher(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx){
    if(type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE){
        sem_t* sem = (sem_t*)zoo_get_context(zh);
        sem_post(sem);
    }
}

void ZkClient::start(){
    string ip = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");

    string connstr = ip + ":" + port;//zk声明的固定格式
 
    //异步连接的
    m_zkhandle = zookeeper_init(connstr.c_str(),my_watcher,30,nullptr,nullptr,0);
    if(m_zkhandle == nullptr){
        cout << "zookeeper_init error" <<endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem,0,0);
    zoo_set_context(m_zkhandle,&sem);

    sem_wait(&sem);
    cout << "zookeeper_init success!" << endl;
}

void ZkClient::create(const char* path,const char* value,int valuelen,int state){//定义时不需要默认参数
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);

    int flag = zoo_exists(m_zkhandle,path,0,nullptr);//先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
    if(flag == ZNONODE){
        // 创建指定path的znode节点了
        flag = zoo_create(m_zkhandle,path,value,valuelen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag == ZOK){
            cout << "znode create success... path:" << path << endl;
        }else{
            cout << "flag:" << flag << endl;
			cout << "znode create error... path:" << path << endl;
			exit(EXIT_FAILURE);
        }
    }
}
string ZkClient::getData(const char* path){
    char buf[64];
    int buf_len = sizeof(buf);
    int flag = zoo_get(m_zkhandle,path,0,buf,&buf_len,nullptr);
    if (flag != ZOK)
	{
		cout << "get znode error... path:" << path << endl;
		return "";
    }
    return buf;
}