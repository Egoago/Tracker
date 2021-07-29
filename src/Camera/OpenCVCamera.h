#pragma once

#include "Camera.h"
#include <opencv2/opencv.hpp>

namespace tr
{
	class OpenCVCamera : public Camera {
	private:
		void* cap;
		cv::Mat frame;
	public:
		OpenCVCamera(int id = 0);
		~OpenCVCamera();
		virtual char* getNextFrame() override;
		virtual double getFPS() const override;
		virtual int getFormat() const override;
	};
}

