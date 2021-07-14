#pragma once
#include "Geometry.h"
#include <opencv2/core/mat.hpp>
#include <glm/mat4x4.hpp>

struct DirectedEdge {
    glm::vec3 a, b;
    DirectedEdge(glm::vec3 a = glm::vec3(), glm::vec3 b = glm::vec3()) : a(a), b(b) {}

    DirectedEdge& flip() {
        const glm::vec3 c = a;
        a = b;
        b = c;
        return *this;
    }

    bool operator==(const DirectedEdge& other) {
        return (a == other.a) && (b == other.b);
    }
};

class ModelEdgeDetector
{
	const Geometry& geometry;
	std::vector<DirectedEdge> edgePairs;
public:
	ModelEdgeDetector(const Geometry& geometry);
    std::vector<Edge<>> detectOutlinerEdges(cv::Mat& edgeMap, cv::Mat& out, glm::mat4 MVP);

};

