#include "ModelEdgeDetector.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc.hpp>

std::vector<DirectedEdge> detectEdgePairs(const Geometry& geometry) {
    std::vector<DirectedEdge> pairs(geometry.getIndecesCount());
    unsigned int* indices = (unsigned int*)geometry.getIndices();
    Vertex* vertices = (Vertex*)geometry.getVertices();
    //TODO implement a more sohpisticated algorithm
    //like leave out edges on same triangle
    //or match triangles
    size_t pairCount = 0, singleCount = 0;
    for (size_t i = 0; i < geometry.getIndecesCount()/3; i++) {
        for (size_t k = 0; k < 3; k++) {
            DirectedEdge edge = DirectedEdge(vertices[indices[3*i + k]],    //1.
                                             vertices[indices[3*i + (1+k)%3]]);
            bool foundPair = false;
            for (size_t l = pairCount; l < (singleCount + pairCount); l++) {//2.
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
    size_t sum = 0;
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

std::vector<Edge> ModelEdgeDetector::detectOutlinerEdges(cv::Mat& depthMap, cv::Mat& out, glm::mat4 MVP)
{
    std::vector<Edge> edges;
    //depthMap = cv::Scalar::all(0);
    for (size_t i = 0; i < edgePairs.size(); i += 2) {
        glm::vec3 a = edgePairs[i+1].a.position;
        glm::vec3 b = edgePairs[i+1].b.position;
        glm::vec2 p1 = screenToNDC(MVP * glm::vec4(a,1.0));
        glm::vec2 p2 = screenToNDC(MVP * glm::vec4(b, 1.0));
        cv::Point2f P1(p1.x * depthMap.cols, p1.y * depthMap.rows);
        cv::Point2f P2(p2.x * depthMap.cols, p2.y * depthMap.rows);
        if (testLine(P1, P2, depthMap)) {
            edges.push_back(Edge(a, b));
            cv::line(out, P1,P2, cv::Scalar(255.0, 255.0));
        }
    }
    return edges;
}
