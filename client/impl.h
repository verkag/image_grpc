#include <memory>
#include <string>
#include <unordered_map>
#include <grpcpp/grpcpp.h>


#include <image_process.grpc.pb.h>
#include <image_process.pb.h>
#include <pqxx/pqxx> 


class ImageProcessClient {
public: 
    ImageProcessClient(std::shared_ptr<grpc::Channel> ch);

    void Transform(const std::string& name, const std::string& transformation);
    void LoadImage(const std::string& name, const std::string& path); 
    void Remove(const std::string& name);
    void List();
private: 
    std::unique_ptr<ImageService::Stub> stub_;
    std::unordered_map<std::string, std::string> storage_;
    pqxx::connection conn_;
};
