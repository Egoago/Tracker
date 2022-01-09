#include "MultiFlashDetector.hpp"
//TODO remove logging
#include "../Misc/Log.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/imgproc.hpp>

tr::MultiFlashDetector::MultiFlashDetector(const uint flashCount) :
	flashCount(flashCount) {
	I = new FloatMat[flashCount];
	R = new FloatMat[flashCount];
}

tr::MultiFlashDetector::~MultiFlashDetector() {
	delete[] I;
	delete[] R;
}

cv::Mat tr::MultiFlashDetector::getDepthMap(const std::vector<cv::Mat>& flashImages) {
	tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
	assert(flashImages.size() == flashCount+1);
	uint k = 0u;
	cv::Mat cvtmp;
	for (const auto& flashImage : flashImages) {
		cvtmp = flashImage.clone();
		if (cvtmp.type() != CV_32F)
			cvtmp.convertTo(cvtmp, CV_32F);
		if (cvtmp.channels() != 1)
			cv::cvtColor(cvtmp, cvtmp, cv::COLOR_BGR2GRAY);
		const float sigmaScale = 5.6f;
		const float scale = 0.8f;
		const float sigma = sigmaScale / scale;
		//cv::bilateralFilter(flashImage, cvtmp, -1, 40, 6);
		//cv::GaussianBlur(flashImage, cvtmp, cv::Size(5, 5),sigma,sigma);
		//cv::resize(cvtmp, cvtmp, cv::Size(0, 0), scale, scale, cv::InterpolationFlags::INTER_NEAREST);
		//cv::pyrDown(flashImage, cvtmp, cv::Size((float)(flashImage.cols + 1) * scale, (float)(flashImage.rows + 1) * scale));
		/*for (int i = 1; i < 4; i+=2) {
			cv::bilateralFilter(cvtmp, bil, i, i*2, i/2.0);
			cv::medianBlur(cvtmp, median, i);
			cv::blur(cvtmp, box, cv::Size(i, i));
			cv::GaussianBlur(cvtmp, gaus, cv::Size(i, i), 0, 0);
			Logger::drawFrame(&box, "box");
			Logger::drawFrame(&bil, "bil");
			Logger::drawFrame(&median, "median");
			Logger::drawFrame(&gaus, "gaus");
			cv::waitKey(1000000);
		}*/
		Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> etmp;
		cv::cv2eigen(cvtmp, etmp);
		if (k == 0u)
			ambient = etmp; //Eigen::Map<BinaryMat, Eigen::RowMajor>(tmp.data, height, width);
		else
			I[k - 1] = FloatMat::Zero(etmp.rows(), etmp.cols()).max(etmp.array() - ambient);
		k++;
	}
	eigen2cv(Eigen::Matrix<float, -1, -1>(ambient), cvtmp);
	Logger::drawFrame(&cvtmp, "ambient");
	eigen2cv(Eigen::Matrix<float, -1, -1>(I[6]), cvtmp);
	Logger::drawFrame(&cvtmp, "left");
	Imax = FloatMat::Zero(ambient.rows(), ambient.cols());
	for (uint k = 0; k < flashCount; k++)
		Imax = Imax.max(I[k]);
	eigen2cv(Eigen::Matrix<float, -1, -1>(Imax), cvtmp);
	Logger::drawFrame(&cvtmp, "max");
	for (uint k = 0; k < flashCount; k++)
		R[k] = I[k] / Imax;
	eigen2cv(Eigen::Matrix<float, -1, -1>(R[6]), cvtmp);
	Logger::drawFrame(&cvtmp, "ratio left");
	tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
	return cv::Mat();
}
