#include "CannyDetector.h"
#include <opencv2/imgproc.hpp>

using namespace cv;

void CannyDetector::detectEdges(cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const {
    blur(img, img, Size(3, 3));
    blur(img, img, Size(3, 3));
    Canny(img, img, 60,180,3);
    //TODO implement edge extraction
    throw "Not implemented";
}
