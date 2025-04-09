#include <image_process.grpc.pb.h>
#include <image_process.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

class ImageProcessImpl final : public imageservice::ImageService::CallbackService { 
public: 
    ::grpc::ServerUnaryReactor* UploadAndProcess(::grpc::CallbackServerContext* ctx, const imageservice::ImageRequest* req, imageservice::ImageResponse* res);
};
