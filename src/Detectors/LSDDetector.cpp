#include "LSDDetector.hpp"
#include <opencv2/imgproc.hpp>
extern "C" {
#include "lsd.h"
}
//TODO remove debug
#include <opencv2/highgui.hpp>
#include "../Misc/Log.hpp"

//TODO namespace fighting
using namespace cv;
using namespace std;
using namespace tr;

//Doesn't work as intended
void LSDDetector::detectEdges(const cv::Mat& img, std::vector<Edge<vec2f>>& edges) const {
    Logger::logProcess(__FUNCTION__);
    const int width = img.cols, height = img.rows;
    Mat doubleImg;
    img.convertTo(doubleImg, CV_64F);
    int lineCount = 0;
    double* lines = lsd_scale(&lineCount, doubleImg.ptr<double>(), width, height, 0.5);
    for (int i = 0; i < lineCount; i++) {
        const vec2f a( lines[i * 7 + 0] / width,
                lines[i * 7 + 1] / height);
        const vec2f b( lines[i * 7 + 2] / width,
                lines[i * 7 + 3] / height);
        edges.push_back(Edge<vec2f>(a, b));
    }
    Logger::logProcess(__FUNCTION__);
}
