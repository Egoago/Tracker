#include "ModelEdgeDetector.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc.hpp>

using namespace tr;

ModelEdgeDetector::ModelEdgeDetector(const Geometry& geometry) : geometry(geometry)
{
}

glm::vec2 screenToNDC(glm::vec4 screen) {
    screen = screen / screen.w;
    //screen = screen / screen.z;
    screen = (screen + 1.0f) / 2.0f;
    return glm::vec2(screen.x, 1.0f - screen.y);
}

bool testPoint(cv::Point b, cv::Mat& depthMap) {
    return depthMap.at<bool>(b);
}

bool testArea(cv::Point b, cv::Mat& depthMap) {
    if (b.x - 1 < 0 || b.y - 1 < 0 ||
        b.x + 1 > depthMap.cols || b.y + 1 > depthMap.rows)
        return false;
    unsigned int sum = 0;
    sum += testPoint(b, depthMap);
    sum += testPoint(b + cv::Point(0, 1), depthMap);
    sum += testPoint(b + cv::Point(1, 0), depthMap);
    sum += testPoint(b + cv::Point(0, -1), depthMap);
    sum += testPoint(b + cv::Point(-1, 0), depthMap);
    return sum > 1;
}

bool testLine(cv::Point a, cv::Point b, cv::Mat& depthMap) {
    return (testArea(a, depthMap) &&
            testArea((2*a+b)/3, depthMap) &&
            testArea((a+2*b)/3, depthMap) &&
            testArea(b, depthMap));
}

std::vector<Edge<>> ModelEdgeDetector::detectOutlinerEdges(cv::Mat& depthMap, cv::Mat& out, glm::mat4 MVP)
{
    std::vector<Edge<>> edges;
    //depthMap = cv::Scalar::all(0);
    for (unsigned int i = 0; i < edgePairs.size(); i += 2) {
        glm::vec3 a = edgePairs[i+1].a;
        glm::vec3 b = edgePairs[i+1].b;
        glm::vec2 p1 = screenToNDC(MVP * glm::vec4(a,1.0));
        glm::vec2 p2 = screenToNDC(MVP * glm::vec4(b, 1.0));
        cv::Point P1((int)(p1.x * (float)depthMap.cols), (int)(p1.y * (float)depthMap.rows));
        cv::Point P2((int)(p2.x * (float)depthMap.cols), (int)(p2.y * (float)depthMap.rows));
        if (cv::clipLine(cv::Size(depthMap.cols, depthMap.rows), P1, P2)) {
            if (testLine(P1, P2, depthMap)) {
                edges.push_back(Edge<>(a, b));
                cv::line(out, P1, P2, cv::Scalar(255.0, 255.0));
            }
        }
    }
    return edges;
}
