# 把当前目录下的所有源文件文件名字添加到SRC_LIST中
#aux_source_directory(. SRC_LIST)
set(SRC_LIST
MprpcApplication.cpp
MprpcProvider.cpp
MprpcConfig.cpp  
MprpcChannel.cpp
MprpcController.cpp
zookeeperutil.cpp
rpcheader.pb.cc
)
# 根据SRC_LIST中的源文件编译生成动态库mprpc
add_library(mprpc ${SRC_LIST})

target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)
