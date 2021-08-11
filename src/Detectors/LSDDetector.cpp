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
using namespace glm;
using namespace tr;

//Doesn't work as intended
void LSDDetector::detectEdges(const cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const {
    Logger::logProcess(__FUNCTION__);
    Mat doubleImg;
    img.convertTo(doubleImg, CV_64F);
    int lineCount = 0;
    double* lines = lsd_scale(&lineCount, doubleImg.ptr<double>(), doubleImg.cols, doubleImg.rows, 0.5);
    for (int i = 0; i < lineCount; i++) {
        const vec2 a( lines[i * 7 + 0],
                lines[i * 7 + 1]);
        const vec2 b( lines[i * 7 + 2],
                lines[i * 7 + 3]);
        edges.push_back(Edge<vec2>(a, b));
    }
    //TODO remove logging
    Mat image(Size(doubleImg.cols, doubleImg.rows), CV_8U, Scalar(0));
    for (const auto& edge : edges) {
        Point A((int)edge.a.x, (int)edge.a.y),
              B((int)edge.b.x, (int)edge.b.y);
        line(image, A, B, Scalar(255), 1, LINE_8);
    }
    imshow("edges", image);
    waitKey(1);
    Logger::logProcess(__FUNCTION__);
}
