#pragma once

#include "Camera.hpp"
#include <opencv2/core/mat.hpp>

namespace tr {
	class OpenCVCamera : public Camera {
	private:
		void* cap;
		cv::Mat frame;
	public:
		OpenCVCamera(int id = 0);
		~OpenCVCamera();
		virtual char* getNextFrameData() override;
		virtual double getFPS() const override;
		virtual int getFormat() const override;
	};
}

