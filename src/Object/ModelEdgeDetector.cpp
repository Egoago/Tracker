#include "ModelEdgeDetector.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc.hpp>

std::vector<Edge<>> detectEdgePairs(const Geometry& geometry) {
    std::vector<Edge<>> pairs(geometry.getIndecesCount());
    const unsigned int* indices = geometry.getIndices();
    const glm::vec3* vertices = geometry.getVertices();
    //TODO implement a more sohpisticated algorithm
    //like leave out edges on same triangle
    //or match triangles
    unsigned int pairCount = 0, singleCount = 0;
    for (unsigned int i = 0; i < geometry.getFacesCount(); i++) {
        for (unsigned int k = 0; k < 3; k++) {
            Edge<> edge = Edge<>(vertices[indices[3*i + k]],    //1.
                                vertices[indices[3*i + (1+k)%3]]);
            bool foundPair = false;
            for (unsigned int l = pairCount; l < (singleCount + pairCount); l++) {//2.
                if (pairs[2 * l] == edge ||                                 //3.
                    pairs[2 * l] == edge.flip()) {
                    pairs[2 * pairCount + 1] = pairs[2 * l];                //4.
                    pairs[2 * l] = pairs[2 * pairCount];                    //5.
                    pairs[2 * pairCount] = edge;                            //6.
                    pairCount++;                                            //7.
                    singleCount--;
                    foundPair = true;
                    break;
                }
            }
            if (!foundPair) {
                pairs[2 * (singleCount + pairCount)] = edge;
                singleCount++;
            }
        }
    }
    std::cout << "edges: " << geometry.getIndecesCount() << std::endl;
    std::cout << "pairs: " << pairCount << std::endl;
    std::cout << "missed: " << singleCount << std::endl;
    return pairs;
}

ModelEdgeDetector::ModelEdgeDetector(const Geometry& geometry) : geometry(geometry)
{
    edgePairs = detectEdgePairs(geometry);
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
