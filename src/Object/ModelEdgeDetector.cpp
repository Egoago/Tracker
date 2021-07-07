#include "ModelEdgeDetector.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc.hpp>

std::vector<DirectedEdge> detectEdgePairs(Geometry& geometry) {
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

ModelEdgeDetector::ModelEdgeDetector(Geometry& geometry) : geometry(geometry)
{
    edgePairs = detectEdgePairs(geometry);
}

glm::vec2 screenToNDC(glm::vec4 screen) {
    screen = screen / screen.w;
    //screen = screen / screen.z;
    screen = (screen + 1.0f) / 2.0f;
    return glm::vec2(screen.x, 1.0f - screen.y);
}

bool testPoint(cv::Point b, cv::Mat& edgeMap) {
    if (b.x - 1 < 0 || b.y - 1 < 0 ||
        b.x + 1 > edgeMap.cols || b.y + 1 > edgeMap.rows)
        return false;
    size_t sum = 0;
    sum += edgeMap.at<bool>(b);
    sum += edgeMap.at<bool>(b+cv::Point(0,1));
    sum += edgeMap.at<bool>(b+cv::Point(1,0));
    sum += edgeMap.at<bool>(b+cv::Point(0,-1));
    sum += edgeMap.at<bool>(b+cv::Point(-1,0));
    return sum > 1;
}

bool testLine(cv::Point a, cv::Point b, cv::Mat& edgeMap) {
    return (testPoint(a, edgeMap) &&
            testPoint((2*a+b)/3, edgeMap) &&
            testPoint((a+2*b)/3, edgeMap) &&
            testPoint(b, edgeMap));
}

void ModelEdgeDetector::detectOutlinerEdges(cv::Mat& edgeMap, cv::Mat& out, glm::mat4 MVP)
{
    cv::Mat edges;
    //depthMap = cv::Scalar::all(0);
    for (size_t i = 0; i < edgePairs.size(); i += 2) {
        Vec3 a = edgePairs[i+1].a.position;
        Vec3 b = edgePairs[i+1].b.position;
        glm::vec2 p1 = screenToNDC(MVP * glm::vec4(a.x,a.y,a.z,1.0));
        glm::vec2 p2 = screenToNDC(MVP * glm::vec4(b.x, b.y, b.z, 1.0));
        cv::Point P1(p1.x * edgeMap.cols, p1.y * edgeMap.rows);
        cv::Point P2(p2.x * edgeMap.cols, p2.y * edgeMap.rows);
        if(testLine(P1, P2, edgeMap))
            cv::line(out, P1,P2, cv::Scalar(255.0, 255.0));
    }
}
