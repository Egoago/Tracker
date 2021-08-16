#include "Log.hpp"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <Windows.h>	//TODO only for windows
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace tr;

//hidden class to preserve includes in cpp
class InternalLogger {
	typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
	static std::unordered_map<std::string, TimePoint> runningProcesses;
	static std::unordered_set<std::string> warnings, errors;
	static void* hConsole;
	static void printTabs();
	static int defaultColor;
	InternalLogger() {} //disable instantiation
	static std::ostream& os;
public:
	static bool logging;
	static void logProcess(const std::string& processName);

	static void log(const std::string& message, bool noEndl = false);
	static void warning(const std::string& message, bool repeat = false);
	static void error(const std::string& message);
};

void* InternalLogger::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
unordered_map<string, InternalLogger::TimePoint> InternalLogger::runningProcesses;
unordered_set<string> InternalLogger::warnings;
unordered_set<string> InternalLogger::errors;
bool InternalLogger::logging = true;
int InternalLogger::defaultColor = 7;
ostream& InternalLogger::os = std::cout;

void InternalLogger::printTabs() {
	os << string(runningProcesses.size(), '\t');
}

void Logger::logProcess(const string& processName) {
	InternalLogger::logProcess(processName);
} 

void InternalLogger::logProcess(const string& processName) {
	using namespace std::chrono;
	if (!logging) return;
	if (runningProcesses.count(processName) == 0) {
		log(processName);
		runningProcesses.insert({ processName, high_resolution_clock::now() });
	}
	else {
		const TimePoint start = runningProcesses.at(processName);
		const TimePoint stop = high_resolution_clock::now();
		long long duration = duration_cast<nanoseconds>(stop - start).count();
		runningProcesses.erase(processName);
		log("Finished in: " 
			+ std::to_string(duration / (int)1e9) + "s "
			+ std::to_string(duration % (int)1e9 / (int)1e6) + "ms "
			+ std::to_string(duration % (int)1e6 / (int)1e3) + "us "
			+ std::to_string(duration % (int)1e3) + "ns."
		);
	}
}

void Logger::log(const string& message, bool noEndl) {
	InternalLogger::log(message,  noEndl);
}
void InternalLogger::log(const string& message, bool noEndl) {
	if (!logging) return;
	SetConsoleTextAttribute(hConsole, 8);
	InternalLogger::printTabs();
	os << message;
	if (!noEndl)
		os << endl;
	SetConsoleTextAttribute(hConsole, defaultColor);
}

void Logger::warning(const string& message, bool repeat) {
	InternalLogger::warning(message, repeat);
}

void InternalLogger::warning(const string& message, bool repeat) {
	if (!logging) return;
	if (warnings.count(message) != 0) {
		if(!repeat) return;
	}
	else warnings.insert(message);
	SetConsoleTextAttribute(hConsole, 14);
	InternalLogger::printTabs();
	os << "[WARNING] ";
	os << message << endl;
	SetConsoleTextAttribute(hConsole, defaultColor);
}

void Logger::error(const string& message) {
	InternalLogger::error(message);
}

void tr::Logger::drawFrame(unsigned int width, unsigned int height, void* data, int OpenCVMatType, const char* windowName) {
	const cv::Mat frame((int)height, (int)width, OpenCVMatType, data);
	drawFrame(&frame, windowName);
}

void tr::Logger::drawFrame(const void* cvMat, const char* windowName, const float scale) {
	cv::Mat frame = ((cv::Mat*)cvMat)->clone();
	if (frame.type() == CV_32F || frame.type() == CV_64F)
		cv::normalize(frame, frame, 0, 1, cv::NORM_MINMAX);
	cv::namedWindow(windowName, cv::WINDOW_NORMAL);
	cv::resizeWindow(windowName, cv::Size((int)(frame.cols * scale), (int)(frame.rows* scale)));
	cv::imshow(windowName, frame);
	log(windowName + std::string(" drawn. Press any key to continue..."));
	cv::waitKey(10000000);
}

void InternalLogger::error(const string& message) {
	if (errors.count(message) != 0) return;
	errors.insert(message);
	SetConsoleTextAttribute(hConsole, 12);
	InternalLogger::printTabs();
	os << "[ERROR] ";
	os << message << endl;
	SetConsoleTextAttribute(hConsole, defaultColor);
}
