#include "CannyDetector.h"
#include <opencv2/imgproc.hpp>

using namespace cv;

std::vector<Edge<glm::vec2>> CannyDetector::detectEdges(cv::Mat& img) const
{
    blur(img, img, Size(3, 3));
    blur(img, img, Size(3, 3));
    Canny(img, img, 60,180,3);
    return std::vector<Edge<glm::vec2>>();
}
