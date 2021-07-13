#include "LSDDetector.h"
#include <opencv2/imgproc.hpp>
extern "C" {
#include "lsd.h"
}

using namespace cv;
using namespace std;
using namespace glm;

vector<Edge<vec2>> LSDDetector::detectEdges(cv::Mat& img) const
{
    int lineCount = 0;
    Mat doubleImg;
    img.convertTo(doubleImg, CV_64F);
    double* lines = lsd_scale(&lineCount, (double*)doubleImg.data, img.cols, img.rows, scale);
    float s = 0.4f;
    //===================
    img = Scalar::all(255.0);
    //===================
    vector<Edge<vec2>> edges;
    for (int i = 0; i < lineCount; i++) {
        vec2 a(lines[i * 7] * img.cols * s, lines[i * 7 + 1] * img.cols * s);
        vec2 b(lines[i * 7 + 2] * img.cols * s, lines[i * 7 + 3] * img.cols * s);
        Point A(a.x, a.y), B(b.x, b.y);
        edges.push_back(Edge<vec2>(a, b));
        line(img, A, B, Scalar(0.0), 1, FILLED, LINE_8);
    }
    return edges;
}
