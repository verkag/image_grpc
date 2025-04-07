// OpenCV headers
#include <string>
#include <opencv2/opencv.hpp>


class ImageTransformator {
public: 
    ImageTransformator(const std::string& data, const std::string& transformation);  

    std::string get_result(); 
private:
    void apply_transformation();
    void resize(int width, int height);
    void rotate(double angle);

    // think about caching 
    cv::Mat img_; 
    std::string transformation_;
};







