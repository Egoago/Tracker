#pragma once
#include <vector>
#include <initializer_list>
#include <memory>
#include "../Misc/Base.hpp"
#include "../Math/Tensor.hpp"
#include "../Math/Edge.hpp"
#include "../Detectors/EdgeDetector.hpp"

//TODO reorganise file hierarchy
namespace tr {
	class DistanceTensor {
	private:
		const uint q;
		std::unique_ptr<EdgeDetector> edgeDetector;
		const uint height, width; //height has to be declared first!
		const float maxCost; //height and weight has to be declared first!

		//temp buffers
		std::vector<Edge<vec2f>>* quantizedEdges;
		Tensor<float> buffers;

		bool front;

		void directedDistanceTransform();
		void distanceTransformFromEdges(const std::vector< Edge<vec2f>>& edges);
		void gaussianBlur();
		double interpolate(const std::initializer_list<double>& indices) const;
		double round(const std::initializer_list<double>& indices) const;
		double sample(const std::initializer_list<double>& indices) const;
	public:
		DistanceTensor(const float aspect = 1.0f);
		~DistanceTensor() {
			delete[] quantizedEdges;
		};
		void setFrame(const cv::Mat& nextFrame);
		double evaluate(const double coordinates[3], double partialDerivatives[3] = nullptr) const;
		double at(const uint indices[3]) const;
		inline double operator()(const std::initializer_list<uint>& indices) const { return double(buffers.at(indices)); }
	};
}

