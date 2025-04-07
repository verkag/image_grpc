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



ImageTransformator::ImageTransformator(const std::string& data, const std::string& transformation) : transformation_(transformation) {
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


// roate:90 resize:300x400  greyscale
void ImageTransformator::apply_transformation() {
    std::stringstream ss(transformation_);
    std::string type;
    std::getline(ss, type, ':');

    if (type == "rotate") {
        double angle;
        ss >> angle;
        return rotate(angle);
    }
    else if (type == "resize") {
        int w, h;
        char x; 
        ss >> w >> x >> h;
        return resize(w, h);
    }
    else {
        LOG(INFO) << "match not found";
        return;
    }
}

