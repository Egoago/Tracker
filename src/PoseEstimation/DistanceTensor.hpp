#pragma once
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
		const real maxCost;
		const uint q;
		std::unique_ptr<EdgeDetector> edgeDetector;

		const uint width, height;

		//temp buffers
		std::vector<Edge<glm::vec2>>* quantizedEdges;
		Tensor<real> buffers;

		bool front;

		void directedDistanceTransform();
		void distanceTransformFromEdges(const std::vector< Edge<glm::vec2>>& edges);
		void gaussianBlur();
		real interpolate(const std::initializer_list<real>& indices) const;
	public:
		DistanceTensor(uint width, uint height);
		~DistanceTensor() {
			delete[] quantizedEdges;
		};
		void setFrame(const cv::Mat& nextFrame);
		real Evaluate(const real coordinates[3], real partialDerivatives[3] = nullptr) const;
		bool checkIndices(const uint indices[3]) const;
		real at(const uint indices[3]) const;
		inline real operator()(const std::initializer_list<uint>& indices) const { return buffers.at(indices); }
	};
}

