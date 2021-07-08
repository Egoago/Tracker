#pragma once
#include "Geometry.h"
#include <opencv2/core/mat.hpp>
#include <glm/mat4x4.hpp>

struct DirectedEdge {
    Vertex a, b;
    DirectedEdge(Vertex a = Vertex(), Vertex b = Vertex()) : a(a), b(b) {}

    DirectedEdge& flip() {
        Vertex c = a;
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
	Geometry& geometry;
	std::vector<DirectedEdge> edgePairs;
public:
	ModelEdgeDetector(Geometry& geometry);
    std::vector<Edge> detectOutlinerEdges(cv::Mat& edgeMap, cv::Mat& out, glm::mat4 MVP);

};

