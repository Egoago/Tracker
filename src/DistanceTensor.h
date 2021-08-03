#pragma once
#include <opencv2/core/mat.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include "Misc/ConfigParser.h"
#include "Detectors/EdgeDetector.h"

//TODO rename + reorganise file hierarchy
namespace tr {
	class DistanceTensor {
	private:
		static ConfigParser config;
		const float maxCost;
		const unsigned int q;
		const EdgeDetector* edgeDetector;

		const unsigned int width, height;

		//temp buffers
		std::vector<Edge<glm::vec2>>* quantizedEdges;
		cv::Mat** buffers;
		bool front;
		cv::Mat tmp;
		float* costs;

		void directedDistanceTransform();
		void distanceTransformFromEdges(const std::vector< Edge<glm::vec2>>& edges);
		void gaussianBlur();
	public:
		DistanceTensor(unsigned int width, unsigned int height);
		~DistanceTensor() {
			delete edgeDetector;
			delete[] costs;
		};
		void setFrame(const cv::Mat& nextFrame);
		float getDist(const glm::vec2 uv, const float angle) const;
	};
}

