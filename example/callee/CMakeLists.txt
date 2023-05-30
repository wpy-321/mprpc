#aux_source_directory(. SRC_LIST)
# 把用到的源文件名添加到SRC_LIST中
set(SRC_LIST
userserice.cpp
../user.pb.cc
)

# 根据SRC_LIST中的源文件生成可执行文件provider
add_executable(provider ${SRC_LIST})
# 生成的可执行文件得链接使用的库
target_link_libraries(provider mprpc protobuf)