find_package(Protobuf CONFIG REQUIRED) 
find_package(gRPC CONFIG REQUIRED) 
#find_package(Threads) 


add_library(protos SHARED "${CMAKE_CURRENT_LIST_DIR}/proto/image_process.proto")
target_link_libraries(protos PUBLIC 
            protobuf::libprotobuf
            gRPC::grpc
            gRPC::grpc++)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/")
set(PROTO_IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/proto/")

target_include_directories(protos PUBLIC ${PROTO_BINARY_DIR})

#get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(
                TARGET protos 
                IMPORT_DIRS ${PROTO_IMPORT_DIRS}
                PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")

protobuf_generate(
                TARGET protos
                LANGUAGE grpc
                GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
                PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
                IMPORT_DIRS ${PROTO_IMPORT_DIRS}
                PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")

