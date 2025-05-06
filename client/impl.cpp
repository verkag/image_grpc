#include "impl.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include "absl/log/log.h"

static imageservice::Transformation ParseTransformation(const std::string& transform_str) {
    imageservice::Transformation transformation;
    
    std::string type;
    std::istringstream ss(transform_str);
    std::getline(ss, type, ':');
    
    if (type == "rotate") {
        double angle;
        ss >> angle;
        
        transformation.set_type(imageservice::TransformationType::ROTATE);
        imageservice::RotateParams* params = new imageservice::RotateParams();
        params->set_angle(angle);
        transformation.set_allocated_rotate(params);
    }
    else if (type == "resize") {
        int width, height;
        char x;
        ss >> width >> x >> height;
        
        transformation.set_type(imageservice::TransformationType::RESIZE);
        imageservice::ResizeParams* params = new imageservice::ResizeParams();
        params->set_width(width);
        params->set_height(height);
        transformation.set_allocated_resize(params);
    }
    else if (type == "grayscale") {
        transformation.set_type(imageservice::TransformationType::GRAYSCALE);
    }
    else if (type == "blur") {
        double sigma = 1.5;  // Default value
        if (ss.good()) {
            ss >> sigma;
        }
        
        transformation.set_type(imageservice::TransformationType::BLUR);
        imageservice::BlurParams* params = new imageservice::BlurParams();
        params->set_sigma(sigma);
        transformation.set_allocated_blur(params);
    }
    else {
        transformation.set_type(imageservice::TransformationType::UNKNOWN);
        throw std::runtime_error("Unknown transformation type: " + type);
    }
    
    return transformation;
}

static imageservice::ImageRequest MakeRequest(const std::string& name,
                                              const std::vector<uchar> buff,
                                              const std::string& transform_str) {
    imageservice::ImageRequest req;
    req.set_filename(name);
    req.set_image_data(buff.data(), buff.size()); 
    
    *req.mutable_transformation() = ParseTransformation(transform_str);
     
    return req; //GOD please make it RVO
}

ImageProcessClient::ImageProcessClient(std::shared_ptr<grpc::Channel> ch) : 
    stub_(imageservice::ImageService::NewStub(ch)), 
    conn_("dbname=image_service user=verkag password=123zxc") {
    pqxx::work tx(conn_); 
    pqxx::result r = tx.exec("SELECT * FROM paths");
    tx.commit();

    for (const auto& row: r) {
        storage_[row["name"].as<std::string>()] = row["path"].as<std::string>();
    }
}

std::vector<uchar> ImageProcessClient::ReadAndEncode(const std::string& name) {
    LOG(INFO) << "Starting transform for image: " << name;
    
    if (storage_.find(name) == storage_.end()) { // to be refactored 
        LOG(ERROR) << "Image not found in storage: " << name;
        throw std::runtime_error("Image not found");
    }

    LOG(INFO) << "Reading image from: " << storage_[name];
    cv::Mat m = cv::imread(storage_[name], cv::IMREAD_COLOR);
    if (m.empty()) {
        LOG(ERROR) << "Failed to read image: " << storage_[name];
        throw std::runtime_error("Failed to read image");
    }

    std::vector<uchar> buff;
    std::string ext = name.substr(name.size() - 4);
    cv::imencode(ext, m, buff);
    LOG(INFO) << "Image encoded, size: " << buff.size() << " bytes";
    return buff;
}

std::vector<imageservice::ImageMessageChunked> ImageProcessClient::Chunkify(
    const std::vector<uchar>& buff, const std::string& name, const std::string& transformation) {
    LOG(INFO) << "Start Chunkifying";
    std::vector<imageservice::ImageMessageChunked> message_buff;

    imageservice::ImageMessageChunked meta;
    meta.mutable_meta()->set_filename(name);
    *meta.mutable_meta()->mutable_transformation() = ParseTransformation(transformation); 
    message_buff.push_back(meta); 
     
    LOG(INFO) << "Meta is set";
    
    for (size_t offset = 0; offset < buff.size(); offset += CHUNK_SIZE) {
        LOG(INFO) << "offset: " << offset;
        imageservice::ImageMessageChunked temp;
        size_t chunk_size = std::min(CHUNK_SIZE, buff.size() - offset);
        LOG(INFO) << "chunk size: " << chunk_size;
        temp.mutable_chunk()->set_data(reinterpret_cast<const char*>(&buff[offset]), chunk_size);
        message_buff.push_back(std::move(temp));
    }

    return message_buff;
}

void ImageProcessClient::TransformChunked(const std::string& name, const std::string& transformation) {
    std::vector<uchar> buff = ReadAndEncode(name);

    grpc::ClientContext cctx;
    std::shared_ptr<grpc::ClientReaderWriter<imageservice::ImageMessageChunked,
        imageservice::ImageMessageChunked>> stream(stub_->UploadAndProcessChunked(&cctx));
    LOG(INFO) << "stream initialized";
    std::vector<imageservice::ImageMessageChunked> message_buffer = Chunkify(buff, name, transformation);
    LOG(INFO) << "chunkified";

    try {
        for (auto const& mess : message_buffer) {
            stream->Write(mess);
            LOG(INFO) << "chunk sent";
        }
        stream->WritesDone();
        message_buffer.clear();    

        imageservice::ImageMessageChunked temp;
        while (stream->Read(&temp)) {
            LOG(INFO) << "Chunk has been read"; 
            message_buffer.push_back(std::move(temp));
        }

        LOG(INFO) << "All read"; 

        grpc::Status status = stream->Finish();

        if (status.ok()) {
            LOG(INFO) << "Request processed successfully";

            std::string foo;
            imageservice::ImageMessageChunked meta = message_buffer[0];
            foo.reserve(message_buffer.size() * CHUNK_SIZE);
            for (int i = 1; i < message_buffer.size(); i++) {
                foo += message_buffer[i].chunk().data();
            }

            LOG(INFO) << "Response filename: " << meta.meta().filename();
            LOG(INFO) << "Transformation applied: " << meta.meta().transformation().type();

            std::vector<uchar> buff(foo.begin(), foo.end());
            cv::Mat m = cv::imdecode(buff, cv::IMREAD_COLOR);
            std::string ext = name.substr(name.size() - 4);
            cv::imwrite(name.substr(0, name.size() - 4) + "_processed" + ext, m);
            LOG(INFO) << "Processed image saved to: " << name.substr(0, name.size() - 4) + "_processed" + ext;
        } else {
            LOG(ERROR) << "Request failed: " << status.error_message();
            throw std::runtime_error(status.error_message());
        }

    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
    }

}

void ImageProcessClient::Transform(const std::string& name, const std::string& transformation) {
    std::vector<uchar> buff = ReadAndEncode(name); 

    grpc::ClientContext cctx;
    imageservice::ImageRequest req = MakeRequest(name, buff, transformation);
    LOG(INFO) << "Sending request to server...";
    
    imageservice::ImageResponse res;

    grpc::Status status = stub_->UploadAndProcess(&cctx, req, &res);

    if (status.ok()) {
        LOG(INFO) << "Request processed successfully";
        LOG(INFO) << "Response filename: " << res.filename();
        LOG(INFO) << "Response data size: " << res.processed_image_data().size();
        
        std::vector<uchar> buff(res.processed_image_data().begin(), res.processed_image_data().end());
        cv::Mat m = cv::imdecode(buff, cv::IMREAD_COLOR);
        std::string ext = name.substr(name.size() - 4);
        cv::imwrite(name.substr(0, name.size() - 4) + "_processed" + ext, m);
        LOG(INFO) << "Processed image saved to: " << name.substr(0, name.size() - 4) + "_processed" + ext;
    } else {
        LOG(ERROR) << "Request failed: " << status.error_message();
        throw std::runtime_error(status.error_message());
    }
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
