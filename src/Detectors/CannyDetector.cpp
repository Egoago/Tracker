#include "CannyDetector.hpp"
#include <opencv2/imgproc.hpp>
#include "../Misc/Log.hpp"

using namespace cv;
using namespace tr;

void CannyDetector::detectEdges(const cv::Mat& img, std::vector<Edge<vec2f>>& edges) const {
    //TODO make canny configurable
    blur(img, img, Size(3, 3));
    blur(img, img, Size(3, 3));
    Canny(img, img, 60,180,3);
    //TODO implement edge extraction
    {
        Logger::error("Not implemented");
        exit(1);
    }
}
