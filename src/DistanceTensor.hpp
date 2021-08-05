#pragma once
#include <opencv2/core/mat.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <memory>
#include "Misc/ConfigParser.hpp"
#include "Detectors/EdgeDetector.hpp"

//TODO reorganise file hierarchy
namespace tr {
	class DistanceTensor {
	private:
		static ConfigParser config;
		const float maxCost;
		const unsigned int q;
		std::unique_ptr<EdgeDetector> edgeDetector;

		const unsigned int width, height;

		//temp buffers
		std::vector<Edge<glm::vec2>>* quantizedEdges;
		cv::Mat** buffers; //TODO own Tensor instead of opencv
		bool front;
		cv::Mat tmp;
		float* costs;

		void directedDistanceTransform();
		void distanceTransformFromEdges(const std::vector< Edge<glm::vec2>>& edges);
		void gaussianBlur();
	public:
		DistanceTensor(unsigned int width, unsigned int height);
		~DistanceTensor() {
			delete[] costs;
		};
		void setFrame(const cv::Mat& nextFrame);
		float getDist(const glm::vec2 uv, const float angle) const;
		float getDist(const glm::uvec3 index) const;
	};
}

