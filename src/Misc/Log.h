#pragma once
#include <iostream>
#include <unordered_set>
#include <Windows.h>

class Logger {
	static std::unordered_set<std::string> runningProcesses;
	const static HANDLE hConsole;
	static void printTabs();
	Logger() {} //disable instantiation
	static std::ostream& os;
public:
	static bool logging;
	static void logProcess(const std::string& processName);

	static void log(const std::string& message, bool noEndl = false);
	static void warning(const std::string& message);
	static void error(const std::string& message);
};
