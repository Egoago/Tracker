#pragma once
#include <string>

namespace tr {
	class Logger {
		Logger() {} //disable instantiation
	public:
		static bool logging;
		static void logProcess(const std::string& processName);

		static void log(const std::string& message, bool noEndl = false);
		static void warning(const std::string& message, bool repeat = false);
		static void error(const std::string& message);
		static void drawFrame(unsigned int width, unsigned int height, void* data, int OpenCVMatType = 5, const char* windowName = "frame");
		static void drawFrame(const void* cvMat, const char* windowName = "frame");
	};
}
