#pragma once
#include "Geometry.h"
#include <opencv2/core/mat.hpp>
#include <glm/mat4x4.hpp>
#include "Coordinates.h"

class ModelEdgeDetector
{
	const Geometry& geometry;
	std::vector<Edge<>> edgePairs;
public:
	ModelEdgeDetector(const Geometry& geometry);
    std::vector<Edge<>> detectOutlinerEdges(cv::Mat& edgeMap, cv::Mat& out, glm::mat4 MVP);

};

