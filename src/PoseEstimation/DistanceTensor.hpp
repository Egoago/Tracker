#pragma once
#include <opencv2/core/mat.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <memory>
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Base.hpp"
#include "../Math/Tensor.hpp"
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
		Tensor<float> buffers;

		bool front;
		cv::Mat tmp;

		void directedDistanceTransform();
		void distanceTransformFromEdges(const std::vector< Edge<glm::vec2>>& edges);
		void gaussianBlur();
		void calculateDerivatives();

	public:
		DistanceTensor(uint width, uint height);
		~DistanceTensor() {
			delete[] quantizedEdges;
		};
		void setFrame(const cv::Mat& nextFrame);
		float at(const float indices[3], double partialDerivatives[3] = nullptr) const;
		float operator()(const std::vector<real>& indices) const;
	};
}

