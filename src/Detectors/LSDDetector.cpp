#include "LSDDetector.h"
#include <opencv2/imgproc.hpp>
extern "C" {
#include "lsd.h"
}

using namespace cv;
using namespace std;
using namespace glm;

void LSDDetector::detectEdges(cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const {
    int lineCount = 0;
    Mat doubleImg;
    img.convertTo(doubleImg, CV_64F);
    double* lines = lsd_scale(&lineCount, (double*)doubleImg.data, img.cols, img.rows, scale);
    float s = 0.4f;
    //===================
    img = Scalar::all(255.0);
    //===================
    for (int i = 0; i < lineCount; i++) {
        vec2 a(lines[i * 7] * img.cols * s, lines[i * 7 + 1] * img.cols * s);
        vec2 b(lines[i * 7 + 2] * img.cols * s, lines[i * 7 + 3] * img.cols * s);
        Point A((int)a.x, (int)a.y), B((int)b.x, (int)b.y);
        edges.push_back(Edge<vec2>(a, b));
        line(img, A, B, Scalar(0.0), 1, FILLED, LINE_8);
    }
}
