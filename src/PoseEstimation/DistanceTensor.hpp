#pragma once
#include <opencv2/core/mat.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <initializer_list>
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
		bool checkIndices(const std::vector<real>& indices) const;
		inline float operator()(const std::initializer_list<uint>& indices) const { return buffers.at(indices); }
	};
}

