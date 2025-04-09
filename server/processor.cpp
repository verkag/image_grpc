#include "processor.h" 

#include "absl/log/log.h"
#include <vector>


static std::string encode(const cv::Mat& img, const std::string& format=".png") {
    std::vector<uchar> buffer;
    cv::imencode(format, img, buffer);
    return std::string(buffer.begin(), buffer.end());
}

static cv::Mat decode(const std::string& data) {
    std::vector<uchar> buffer(data.begin(), data.end());
    return cv::imdecode(buffer, cv::IMREAD_COLOR);
}

ImageTransformator::ImageTransformator(const std::string& data, const imageservice::Transformation& transformation) 
    : transformation_(transformation) {
    img_ = decode(data);
}

std::string ImageTransformator::get_result() {
    return encode(img_);
}

void ImageTransformator::rotate(double angle) {
    cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point2f(img_.cols / 2.0, img_.rows / 2.0), angle, 1.0);
    cv::warpAffine(img_, img_, rot_mat, img_.size());
}

void ImageTransformator::resize(int width, int height) {
    cv::resize(img_, img_, cv::Size(width, height), cv::INTER_LINEAR);
}

void ImageTransformator::grayscale() {
    cv::cvtColor(img_, img_, cv::COLOR_BGR2GRAY);
    cv::cvtColor(img_, img_, cv::COLOR_GRAY2BGR);  // Convert back to 3 channels for consistency
}

void ImageTransformator::blur(double sigma) {
    cv::GaussianBlur(img_, img_, cv::Size(0, 0), sigma);
}

void ImageTransformator::apply_transformation() {
    LOG(INFO) << "Applying transformation type: " << transformation_.type();
    
    switch (transformation_.type()) {
        case imageservice::TransformationType::ROTATE:
            if (transformation_.has_rotate()) {
                LOG(INFO) << "Rotating image by " << transformation_.rotate().angle() << " degrees";
                rotate(transformation_.rotate().angle());
            } else {
                LOG(ERROR) << "Rotation parameters missing";
            }
            break;
            
        case imageservice::TransformationType::RESIZE:
            if (transformation_.has_resize()) {
                LOG(INFO) << "Resizing image to " << transformation_.resize().width() 
                         << "x" << transformation_.resize().height();
                resize(transformation_.resize().width(), transformation_.resize().height());
            } else {
                LOG(ERROR) << "Resize parameters missing";
            }
            break;
            
        case imageservice::TransformationType::GRAYSCALE:
            LOG(INFO) << "Converting image to grayscale";
            grayscale();
            break;
            
        case imageservice::TransformationType::BLUR:
            if (transformation_.has_blur()) {
                LOG(INFO) << "Blurring image with sigma: " << transformation_.blur().sigma();
                blur(transformation_.blur().sigma());
            } else {
                LOG(INFO) << "Using default blur parameters";
                blur(1.5);  // Default sigma value
            }
            break;
            
        default:
            LOG(ERROR) << "Unknown transformation type: " << transformation_.type();
            break;
    }
}

