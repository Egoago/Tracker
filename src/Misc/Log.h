#pragma once
#include <iostream>
#include <unordered_set>

namespace tr
{
	class Logger {
		static std::unordered_set<std::string> runningProcesses, warnings, errors;
		static void* hConsole;
		static void printTabs();
		static int defaultColor;
		Logger() {} //disable instantiation
		static std::ostream& os;
	public:
		static bool logging;
		static void logProcess(const std::string& processName);

		static void log(const std::string& message, bool noEndl = false);
		static void warning(const std::string& message);
		static void error(const std::string& message);
	};
}
