#include "Log.h"
//TODO only for windows
#include <Windows.h>
//#include <consoleapi2.h>
//#include <WinBase.h>

using namespace std;
using namespace tr;

void* Logger::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
unordered_set<string> Logger::runningProcesses;
unordered_set<string> Logger::warnings;
unordered_set<string> Logger::errors;
bool Logger::logging = true;
ostream& Logger::os = std::cout;

void Logger::printTabs() {
	os << string(runningProcesses.size(), '\t');
}

void Logger::logProcess(const string& processName) {
	if (!logging) return;
	if (runningProcesses.count(processName) == 0) {
		log(processName);
		runningProcesses.insert(processName);
	}
	else {
		runningProcesses.erase(processName);
		log(processName);
	}
}

void Logger::log(const string& message, bool noEndl) {
	if (!logging) return;
	Logger::printTabs();
	os << message;
	if (!noEndl)
		os << endl;
}

void Logger::warning(const string& message) {
	if (!logging) return;
	if (warnings.count(message) != 0) return;
	warnings.insert(message);
	SetConsoleTextAttribute(hConsole, 14);
	Logger::printTabs();
	os << "[WARNING] ";
	os << message << endl;
	SetConsoleTextAttribute(hConsole, 7);
}

void Logger::error(const string& message) {
	if (errors.count(message) != 0) return;
	errors.insert(message);
	SetConsoleTextAttribute(hConsole, 12);
	Logger::printTabs();
	os << "[ERROR] ";
	os << message << endl;
	SetConsoleTextAttribute(hConsole, 7);
}
