#include "Camera.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <random>
#include <iterator>
#include <algorithm>
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Constants.hpp"
#include "../Misc/Log.hpp"

using namespace tr;
using namespace std;
using namespace cv;

Mat tr::Camera::undistort(const Mat& frame) {
	Mat dst;
	cv::undistort(frame, dst, camMtx, distortion);
	return dst;
}

cv::Mat tr::Camera::getNextFrame() const {
	return cv::Mat(Size(width, height), getFormat(), getNextFrameData());
}

vector<Point3f> generateCircleGrid(Size gridSize) {
	const float scale = ConfigParser::instance().getEntry(CONFIG_SECTION_CAMERA, "scale in mms", 20.0f);
	vector<Point3f> grid(gridSize.area());
	for (int x = 0; x < gridSize.width; x++)
		for (int y = 0; y < 4; y++)
			grid[y * gridSize.width + x] = Point3f(x * scale, (2.0f*y + (y % 2)) * scale, 0.0f);
	return grid;
}

vector<Point3f> generateChessboardGrid(Size gridSize) {
	const float scale = ConfigParser::instance().getEntry(CONFIG_SECTION_CAMERA, "scale in mms", 25.0f);
	vector<Point3f> grid(gridSize.area());
	for (int y = 0; y < gridSize.height; y++)
		for (int x = 0; x < gridSize.width; x++)
			grid[y * gridSize.width + x] = Point3f(scale * x, scale * y, 0);
	return grid;
}

vector<Point2f> findCircleMarkers(const Mat nextFrame, const Size gridSize) {
	vector<Point2f> markers;
	markers.reserve(gridSize.area());
	Mat frame = nextFrame;
	if (nextFrame.type() != CV_8U)
		cvtColor(nextFrame, frame, COLOR_BGR2GRAY);
	const bool markersDetected = findCirclesGrid(frame, gridSize, markers, CALIB_CB_ASYMMETRIC_GRID | CALIB_CB_CLUSTERING);
	if (markersDetected) return markers;
	else return vector<Point2f>();
}

vector<Point2f> findChessboardMarkers(const Mat nextFrame, const Size gridSize) {
	vector<Point2f> markers;
	markers.reserve(gridSize.area());
	Mat frame = nextFrame;
	if (nextFrame.type() != CV_8U)
		cvtColor(nextFrame, frame, COLOR_BGR2GRAY);
	const bool markersDetected = findChessboardCorners(frame, gridSize, markers, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
	if (markersDetected) {
		const TermCriteria criteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 40, 1e-5);
		cornerSubPix(frame, markers, Size(11, 11), Size(-1, -1), criteria);
		return markers;
	}
	else return vector<Point2f>();
}

float Camera::calibrate() {
	Logger::logProcess(__FUNCTION__);
	calibrated = false;
	const bool circles = ConfigParser::instance().getEntry(CONFIG_SECTION_CAMERA, "circles", false);
	const vector<uint> size = ConfigParser::instance().getEntries<uint>(CONFIG_SECTION_CAMERA, "grid", { 9u,6u });
	const Size imSize(width, height), gridSize(size[0], size[1]);
	const vector<Point3f> grid = circles ? generateCircleGrid(gridSize)
										 : generateChessboardGrid(gridSize);
	const uint sampleCount = 30u;
	vector<vector<Point2f>> foundMarkers;
	foundMarkers.reserve(sampleCount * 3);
	double error;
	while (!calibrated) {
		const auto& markers = circles ? findCircleMarkers(getNextFrame(), gridSize)
									  : findChessboardMarkers(getNextFrame(), gridSize);
		if(markers.size() > 0)
			foundMarkers.push_back(markers);
		if (foundMarkers.size() > sampleCount * 3) {
			vector<vector<Point2f>> p2D;
			vector<vector<Point3f>> p3D(sampleCount, grid);
			sample(foundMarkers.begin(), foundMarkers.end(), back_inserter(p2D),
					sampleCount, mt19937{ random_device{}() });
			Mat R, T;
			error = calibrateCamera(p3D, p2D, imSize, camMtx, distortion, R, T);
			camMtx = getOptimalNewCameraMatrix(camMtx, distortion, imSize, 0, imSize);
			if (error < 0.5) {
				calibrated = true;
				break;
			}
		}
		waitKey(10);
	}
	Logger::logProcess(__FUNCTION__);
	return (float)error;
}