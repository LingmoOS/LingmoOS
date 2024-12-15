cmake_minimum_required(VERSION 3.0)
project(rpctest)

SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
enable_language(ASM)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -std=c++11 -Wall")

# do not find, check system installed protobuf package
if(NOT EXISTS "${PROTOBUF_PROTOC_EXECUTABLE}")
    # 生成转换proto文件
    # protoc --cpp_out=./ sample.proto
    set(PROTO_FILES ${PROJECT_SOURCE_DIR}/sample.pb.cc)
# use the local customize protobuf package
else ()
    PROTOBUF_GENERATE_CPP(PROTO_RPC_SAMPLE_SRC PROTO_RPC_SAMPLE_HDR sample.proto)
    set(PROTO_FILES
        ${PROTO_RPC_SAMPLE_SRC}
        ${PROTO_RPC_SAMPLE_HDR}
    )
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

message("use PROTOBUF_LIB: ${PROTOBUF_LIB}")

set(PATH_BIN bin)
set(LIBS
    zrpc
)

# server
set(
    server_src
    ${PROJECT_SOURCE_DIR}/server.cpp
    ${PROTO_FILES}
)
add_executable(server ${server_src})
target_link_libraries(server ${LIBS})
install(TARGETS server DESTINATION ${PATH_BIN})

# client
set(
    client_src
    ${PROJECT_SOURCE_DIR}/client.cpp
    ${PROTO_FILES}
)
add_executable(client ${client_src})
target_link_libraries(client ${LIBS})
install(TARGETS client DESTINATION ${PATH_BIN})
