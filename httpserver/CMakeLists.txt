#CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb -Iphread")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 

# 项目信息
project (RemoteMonit)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 指定生成目标
add_executable(RemoteMonit ${DIR_SRCS})

target_link_libraries(RemoteMonit ${OpenCV_LIBS})
target_link_libraries(RemoteMonit ${CMAKE_THREAD_LIBS_INIT})
