#include <image_process.grpc.pb.h>
#include <image_process.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>



class ImageProcessImpl final : public imageservice::ImageService::CallbackService { 
public: 
    ::grpc::ServerUnaryReactor* UploadAndProcess(::grpc::CallbackServerContext* ctx, const imageservice::ImageRequest* req, imageservice::ImageResponse* res) override;
    ::grpc::ServerBidiReactor<::imageservice::ImageMessageChunked, ::imageservice::ImageMessageChunked>* UploadAndProcessChunked(::grpc::CallbackServerContext* ctx) override;
private:
    static constexpr size_t CHUNK_SIZE = 64 * 1024; // 64 KiB
};
