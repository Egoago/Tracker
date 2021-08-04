#include "LSDDetector.hpp"
#include <opencv2/imgproc.hpp>
//TODO remove debug
//#include <opencv2/highgui.hpp>
extern "C" {
#include "lsd.h"
}

//TODO namespace fighting
using namespace cv;
using namespace std;
using namespace glm;
using namespace tr;

//Doesn't work as intended
void LSDDetector::detectEdges(const cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const {
    Mat doubleImg;
    /*img.copyTo(canny);
    blur(canny, canny, Size(5, 5));
    Canny(canny, doubleImg, 50, 100, 5);*/
    //imshow("image", doubleImg);
    img.convertTo(doubleImg, CV_64F);
    //Logger::log("frame: " + std::to_string(doubleImg.cols) + " " + std::to_string(doubleImg.rows));
    int lineCount = 0;
    double* lines = lsd_scale(&lineCount, doubleImg.ptr<double>(), doubleImg.cols, doubleImg.rows, 0.5);
    float s = 1.0f;
    Mat image(Size(doubleImg.cols, doubleImg.rows), CV_8U, Scalar(0));
    for (int i = 0; i < lineCount; i++) {
        //Logger::log(std::to_string(lines[i * 7 + 5]));
        vec2 a( lines[i * 7 + 0]*s,
                lines[i * 7 + 1]*s);
        vec2 b( lines[i * 7 + 2]*s,
                lines[i * 7 + 3]*s);
        //Point A((int)a.x, (int)a.y), B((int)b.x, (int)b.y);
        edges.push_back(Edge<vec2>(a, b));
        //line(image, A, B, Scalar(255), 1, LINE_8);
    }
    //imshow("edges", image);
    //waitKey(1);
}
