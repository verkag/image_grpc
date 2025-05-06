#include <memory>
#include <string>
#include <unordered_map>
#include <grpcpp/grpcpp.h>


#include <image_process.grpc.pb.h>
#include <image_process.pb.h>
#include <pqxx/pqxx> 
#include <opencv2/opencv.hpp>


class ImageProcessClient {
public: 
    ImageProcessClient(std::shared_ptr<grpc::Channel> ch);

    void Transform(const std::string& name, const std::string& transformation);
    void TransformChunked(const std::string& name, const std::string& transformation);
    void LoadImage(const std::string& name, const std::string& path); 
    void Remove(const std::string& name);
    void List();

private: 
    std::vector<uchar> ReadAndEncode(const std::string& name);
    std::vector<imageservice::ImageMessageChunked> Chunkify(const std::vector<uchar>& buff, const std::string& name, const std::string& transformation);

    std::unique_ptr<imageservice::ImageService::Stub> stub_;
    std::unordered_map<std::string, std::string> storage_;
    pqxx::connection conn_;
    static constexpr size_t CHUNK_SIZE = 64 * 1024; // 64 KiB
};
