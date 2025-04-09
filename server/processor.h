// OpenCV headers
#include <string>
#include <opencv2/opencv.hpp>
#include <image_process.grpc.pb.h>
#include <image_process.pb.h>

class ImageTransformator {
public: 
    ImageTransformator(const std::string& data, const imageservice::Transformation& transformation);  
    std::string get_result(); 
    void apply_transformation();

private:
    void resize(int width, int height);
    void rotate(double angle);
    void grayscale();
    void blur(double sigma);

    // think about caching 
    cv::Mat img_; 
    imageservice::Transformation transformation_;
};







