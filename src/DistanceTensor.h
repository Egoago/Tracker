#pragma once
#include "Misc/ConfigParser.h"
#include <opencv2/core/mat.hpp>
#include "Detectors/EdgeDetector.h"

//TODO rename + reorganise file hierarchy
namespace tr {
	class DistanceTensor {
	private:
		static ConfigParser config;
		const float maxCost;
		const unsigned int q;
		std::vector<cv::Mat>& dcdt3;
		const EdgeDetector* edgeDetector;

		//temp buffers
		std::vector<std::vector<Edge<glm::vec2>>> quantizedEdges;
		std::vector<cv::Mat>& other;
		std::vector<cv::Mat> buffer1, buffer2;
		bool first;
		cv::Mat tmp;
		float* costs;

		void directedDistanceTransform();
		void gaussianBlur();
		void swapBuffers();
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

