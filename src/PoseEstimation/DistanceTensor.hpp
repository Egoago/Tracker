#pragma once
#include <opencv2/core/mat.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <memory>
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Base.hpp"
#include "../Math/Edge.hpp"
#include "../Detectors/EdgeDetector.hpp"

//TODO reorganise file hierarchy
namespace tr {
	class DistanceTensor {
	private:
		static ConfigParser config;
		const float maxCost;
		const uint q;
		std::unique_ptr<EdgeDetector> edgeDetector;

		const uint width, height;

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
		DistanceTensor(uint width, uint height);
		~DistanceTensor() {
			delete[] costs;
		};
		void setFrame(const cv::Mat& nextFrame);
		float at(const glm::vec2 uv, const float angle) const;
		float operator()(const std::vector<uint>& indices) const;
	};
}

