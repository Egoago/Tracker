#pragma once
#include <opencv2/core/mat.hpp>

namespace tr {
	class Camera {
	protected:
		int width, height;
		int nBitsPerPixel;
		cv::Mat camMtx, distortion; //for calibration
		bool calibrated = false;
		virtual char* getNextFrameData() const = 0;
	public:
		cv::Mat undistort(const cv::Mat& frame);
		int getWidth() const { return width; }
		virtual ~Camera() {}
		int getHeight() const { return height; }
		int getPixelSize() const { return nBitsPerPixel / 8; }
		cv::Mat getNextFrame() const;
		virtual double getFPS() const = 0;
		virtual int getFormat() const = 0;
		float calibrate();
	};
}

