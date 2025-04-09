#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "absl/log/log.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"

#include "image_process_impl.h"

int main() {
    absl::SetStderrThreshold(absl::LogSeverityAtLeast::kInfo);
    absl::InitializeLog();

    LOG(INFO) << "Starting server...";
    
    std::string svr_address("localhost:5555");  // Changed from localhost to 0.0.0.0 to listen on all interfaces
    ImageProcessImpl service;

    grpc::ServerBuilder builder;
    LOG(INFO) << "Attempting to bind to address: " << svr_address;
    
    
    builder.AddListeningPort(svr_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    builder.SetMaxReceiveMessageSize(20 * 1024 * 1024); 
    builder.SetMaxSendMessageSize(20 * 1024 * 1024); 
    LOG(INFO) << "Building server...";
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    
    if (!server) {
        LOG(ERROR) << "Failed to start server!";
        return 1;
    }
    
    LOG(INFO) << "Server is listening on " << svr_address;
    LOG(INFO) << "Server is ready to accept requests";
    
    server->Wait();
}
