#!/bin/bash

protobuf_dir="${PWD}/../../3rdparty/protobuf"
protoc="${protobuf_dir}/protoc/linux-x86_64/protoc"

filename=${1:-"message.proto"}
cppout=${2:-${PWD}}

if [ -f $protoc ]; then
    echo "Running C++ protocol buffer compiler on ${filename} out: ${cppout}"
    $protoc --cpp_out ${cppout} ${filename}
else
    echo "the protoc is not exist:$protoc"
fi
