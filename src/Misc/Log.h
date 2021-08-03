#pragma once
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <chrono>

namespace tr
{
	class Logger {
		typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
		static std::unordered_map<std::string, TimePoint> runningProcesses;
		static std::unordered_set<std::string> warnings, errors;
		static void* hConsole;
		static void printTabs();
		static int defaultColor;
		Logger() {} //disable instantiation
		static std::ostream& os;
	public:
		static bool logging;
		static void logProcess(const std::string& processName);

		static void log(const std::string& message, bool noEndl = false);
		static void warning(const std::string& message, bool repeat = false);
		static void error(const std::string& message);
	};
}
