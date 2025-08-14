#!/bin/bash

# Define environment variables
PROTOC_PATH=~/.orca/3rdParty/packages/grpc-1.51.1-rev2-linux/grpc/bin/protoc
GRPC_CPP_PLUGIN=~/.orca/3rdParty/packages/grpc-1.51.1-rev2-linux/grpc/bin/grpc_cpp_plugin

# Run the Protocol Buffers compiler
$PROTOC_PATH --cpp_out=build/ --grpc_out=build/ --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN -I proto/ realenv.proto