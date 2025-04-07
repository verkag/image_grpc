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

    std::string svr_address("localhost:5555");
    ImageProcessImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(svr_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    LOG(INFO) << "Server is listening on " << svr_address;
    server->Wait();
}
