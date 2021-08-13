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
		const uint q;
		std::unique_ptr<EdgeDetector> edgeDetector;
		const uint height, width; //height has to be declared first!
		const float maxCost; //height and weight has to be declared first!

		//temp buffers
		std::vector<Edge<glm::vec2>>* quantizedEdges;
		Tensor<float> buffers;

		bool front;

		void directedDistanceTransform();
		void distanceTransformFromEdges(const std::vector< Edge<glm::vec2>>& edges);
		void gaussianBlur();
		real interpolate(const std::initializer_list<real>& indices) const;
		real round(const std::initializer_list<real>& indices) const;
		real sample(const std::initializer_list<real>& indices) const;
	public:
		DistanceTensor(const float aspect = 1.0f);
		~DistanceTensor() {
			delete[] quantizedEdges;
		};
		void setFrame(const cv::Mat& nextFrame);
		real evaluate(const real coordinates[3], real partialDerivatives[3] = nullptr) const;
		real at(const uint indices[3]) const;
		inline real operator()(const std::initializer_list<uint>& indices) const { return real(buffers.at(indices)); }
	};
}

