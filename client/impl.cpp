#include "impl.h"

#include <opencv2/opencv.hpp>
#include <iostream>
static ImageRequest MakeRequest(const std::string& name, const std::vector<uchar> buff, const std::string& transforamtion) {
    ImageRequest req;
    req.set_fileanme(name);
    req.set_image_data(buff.data(), buff.size()); 
    req.set_transformation(transforamtion);
    
    return req; //GOD please make it RVO
}

ImageProcessClient::ImageProcessClient(std::shared_ptr<grpc::Channel> ch) : stub_(ImageService::NewStub(ch)), conn_("dbname=image_service user=verkag password=123zxc") {
    pqxx::work tx(conn_); 
    pqxx::result r = tx.exec("SELECT * FROM paths");
    tx.commit();

    for (const auto& row: r) {
        storage_[row["name"].as<std::string>()] = row["path"].as<std::string>();
    }
}

void ImageProcessClient::Transform(const std::string& name, const std::string& transformation) {
    cv::Mat m = cv::imread(storage_[name], cv::IMREAD_COLOR);

    std::vector<uchar> buff;
    cv::imencode(name.substr(name.size() - 4), m, buff);

    grpc::ClientContext cctx;
    ImageRequest req = MakeRequest(name, buff, transformation);
    ImageResponse res;
    grpc::Status status = stub_->UploadAndProcess(&cctx, req, &res);
    
    std::cout << "all ok" << std::endl;
    // later decode and store in filesystem
}

void ImageProcessClient::LoadImage(const std::string& name, const std::string& path) {
    pqxx::work tx(conn_);
    tx.exec("INSERT INTO paths (name, path) VALUES ($1, $2)", pqxx::params(name, path));
    tx.commit(); 
    storage_[name] = path;
    return; 
}

void ImageProcessClient::Remove(const std::string& name) {
    pqxx::work tx(conn_);
    tx.exec("DELETE FROM paths WHERE name = $1", pqxx::params(name));
    tx.commit();
    
    storage_.erase(name);
}


void ImageProcessClient::List() {
    for (const auto& [k, v] : storage_) {
        std::cout << k << " " << v << std::endl;
    }
}
