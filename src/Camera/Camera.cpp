#include "Camera.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <random>
#include <iterator>
#include <algorithm>
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Constants.hpp"
#include "../Misc/Log.hpp"
#include "../Misc/Base.hpp"

using namespace tr;
using namespace std;
using namespace cv;

CameraParameters getCameraParameters(const Mat& projection, const Size imSize) {
	ConfigParser config = ConfigParser::instance();
	const auto sensorSize = config.getEntries<double>(CONFIG_SECTION_CAMERA, "sensor", { 5.37, 4.04 }); // 1/2.7"
	CameraParameters calibraion;
	double fovx, fovy;
	Point2d c;
	double aspect, focalLength;
	calibrationMatrixValues(projection,
		imSize,
		sensorSize[0],
		sensorSize[1],
		fovx,
		fovy,
		focalLength,
		c,
		aspect);
	calibraion.FOVy = radian(fovy);
	calibraion.aspect = (float)imSize.width / imSize.height;
	calibraion.resolution.x() = imSize.width;
	calibraion.resolution.y() = imSize.height;
	return calibraion;
}

void tr::Camera::load() {
	if(calibrated = loadCalibration())
		calibration = getCameraParameters(projection, Size(width, height));
}

Mat tr::Camera::undistort(const Mat& frame) {
	Mat dst = frame;
	if (calibrated) cv::undistort(frame.clone(), dst, projection, distortion);
	else Logger::warning("Camera is not yet calibrated.");
	return dst;
}

cv::Mat tr::Camera::getNextFrame() {
	return cv::Mat(Size(width, height), getFormat(), getNextFrameData());
}

vector<Point3f> generateCircleGrid(Size gridSize) {
	const float scale = ConfigParser::instance().getEntry(CONFIG_SECTION_CAMERA, "scale in mms", 20.0f);
	vector<Point3f> grid(gridSize.area());
	for (int y = 0; y < gridSize.height; y++)
		for (int x = 0; x < gridSize.width; x++)
			grid[y * gridSize.width + x] = Point3f(y * scale, (2.0f * x + (y % 2)) * scale, 0.0f);
	return grid;
}

vector<Point3f> generateChessboardGrid(Size gridSize) {
	const float scale = ConfigParser::instance().getEntry(CONFIG_SECTION_CAMERA, "scale in mms", 25.0f);
	vector<Point3f> grid(gridSize.area());
	for (int x = 0; x < gridSize.width; x++)
		for (int y = 0; y < gridSize.height; y++)
			grid[y * gridSize.width + x] = Point3f(scale * x, scale * y, 0);
	return grid;
}

vector<Point2f> findCircleMarkers(const Mat nextFrame, const Size gridSize) {
	vector<Point2f> markers;
	markers.reserve(gridSize.area());
	Mat frame = nextFrame.clone();
	if (nextFrame.type() != CV_8U)
		cvtColor(nextFrame, frame, COLOR_BGR2GRAY);
	threshold(frame, frame, 80, 255, THRESH_BINARY);
	const Mat se = getStructuringElement(MORPH_RECT, Size(3, 3));
	erode(frame, frame, se);
	dilate(frame, frame, se);
	SimpleBlobDetector::Params params;
	params.maxArea = ((float)frame.rows * frame.cols) / (gridSize.height * gridSize.width * 2);
	params.minArea = 10;
	params.minConvexity = 0.85f;
	params.minThreshold = 80;
	params.maxThreshold = 230;
	params.thresholdStep = 20;
	params.minInertiaRatio = 0.05f;
	const bool markersDetected = findCirclesGrid(frame, gridSize, markers, CALIB_CB_ASYMMETRIC_GRID, SimpleBlobDetector::create(params));
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

CameraParameters Camera::calibrate(bool reset) {
	Logger::logProcess(__FUNCTION__);
	if (reset) calibrated = false;
	if (!calibrated) {
		const bool circles = ConfigParser::instance().getEntry(CONFIG_SECTION_CAMERA, "circles", false);
		const uvec2 size = circles ? uvec2(4u, 11u) : uvec2(9u,6u);
		const Size imSize(width, height), gridSize(size[0], size[1]);
		const vector<Point3f> grid = circles ? generateCircleGrid(gridSize)
			: generateChessboardGrid(gridSize);
		const uint sampleCount = 40u;
		vector<vector<Point2f>> foundMarkers;
		foundMarkers.reserve(sampleCount * 3);
		const uint stepsBetweenCaptures = 5;
		uint counter = stepsBetweenCaptures;
		while (!calibrated) {
			const Mat frame = getNextFrame();
			if (counter-- == 0) {
				counter = stepsBetweenCaptures;
				const vector<Point2f> markers = circles ? findCircleMarkers(frame, gridSize)
					: findChessboardMarkers(frame, gridSize);
				if (markers.size() > 0) {
					foundMarkers.push_back(markers);
					drawChessboardCorners(frame, gridSize, markers, true);
				}
				if (foundMarkers.size() > sampleCount * 3) {
					Logger::logProcess("calibrating");
					vector<vector<Point2f>> p2D;
					vector<vector<Point3f>> p3D(sampleCount, grid);
					sample(foundMarkers.begin(), foundMarkers.end(), back_inserter(p2D),
						sampleCount, mt19937{ random_device{}() });
					Mat R, T;
					const double error = calibrateCamera(p3D, p2D, imSize, projection, distortion, R, T);
					projection = getOptimalNewCameraMatrix(projection, distortion, imSize, 0, imSize);
					calibration = getCameraParameters(projection, Size(width, height));
					Logger::log("FOVy=" + tr::string(calibration.FOVy));
					calibrated = saveCailbration();
					break;
				}
			}
			cv::imshow("Distorted", frame);
			cv::waitKey(1);
		}
	}
	Logger::logProcess(__FUNCTION__);
	return calibration;
}

CameraParameters tr::Camera::getParameters() {
	if (!calibrated) {
		Logger::warning("Camera is not yet calibrated.");
		return CameraParameters::default();
	}
	return calibration;
}

bool saveDoubleMat(const Mat mat, const std::string& keyName) {
	Mat mat2 = mat.clone();
	if (mat2.type() != CV_64FC1)
		mat.convertTo(mat2, CV_64FC1);
	if (!mat2.isContinuous()) return false;
	const std::vector<double> array = mat2.reshape(1, (int)mat2.total() * mat2.channels());
	ConfigParser::instance().setEntries(CONFIG_SECTION_CAMERA, keyName, array);
	return true;
}

bool tr::Camera::saveCailbration() {
	return saveDoubleMat(projection, "projection") &&
		saveDoubleMat(distortion, "distortion");
}

bool loadDoubleMat(Mat& mat, const std::string& keyName, const Size matSize) {
	if (!ConfigParser::instance().hasEntry(CONFIG_SECTION_CAMERA, keyName)) return false;
	std::vector<double> array = ConfigParser::instance().getEntries<double>(CONFIG_SECTION_CAMERA, keyName);
	if (array.size() != matSize.area()) return false;
	mat = Mat(matSize, CV_64FC1, array.data()).clone();
	return true;
}

bool tr::Camera::loadCalibration() {
	return loadDoubleMat(projection, "projection", Size(3, 3)) &&
		loadDoubleMat(distortion, "distortion", Size(5, 1));
}