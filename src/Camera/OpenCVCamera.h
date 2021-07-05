#pragma once

#include "Camera.h"
#include <opencv2/opencv.hpp>

class OpenCVCamera : public Camera {
private:
	void* cap;
	cv::Mat frame;
public:
	OpenCVCamera(int id = 0);
	~OpenCVCamera();
	virtual char* getNextFrame();
	virtual double getFPS() const;
	virtual int getFormat() const;
};

