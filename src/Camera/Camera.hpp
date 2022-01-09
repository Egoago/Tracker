#pragma once
#include <opencv2/core/mat.hpp>
#include "CameraParameters.hpp"

namespace tr {
	class Camera {
	private:
		bool saveCailbration();
		bool loadCalibration();
	protected:
		int width, height;
		int nBitsPerPixel;
		cv::Mat projection, distortion; //for calibration
		CameraParameters calibration;
		bool calibrated = false;
		virtual char* getNextFrameData() = 0;
		void load();
	public:
		cv::Mat undistort(const cv::Mat& frame);
		inline int getWidth() const { return width; }
		virtual ~Camera() {}
		inline int getHeight() const { return height; }
		inline int getPixelSize() const { return nBitsPerPixel / 8; }
		cv::Mat getNextFrame();
		virtual double getFPS() const = 0;
		virtual int getFormat() const = 0;
		CameraParameters calibrate(bool reset = false);
		CameraParameters getParameters();
		inline bool isCalibrated() { return calibrated; }
	};
}

