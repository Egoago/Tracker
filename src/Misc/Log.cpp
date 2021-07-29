#include "Log.h"
//TODO only for windows
//#include <consoleapi2.h>
//#include <WinBase.h>

using namespace std;
using namespace tr;

const HANDLE Logger::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
unordered_set<string> Logger::runningProcesses;
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
		//printTabs();
		//os << processName << endl;
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
	SetConsoleTextAttribute(hConsole, 14);
	Logger::printTabs();
	os << "[WARNING] ";
	os << message << endl;
	SetConsoleTextAttribute(hConsole, 7);
}

void Logger::error(const string& message) {
	SetConsoleTextAttribute(hConsole, 12);
	Logger::printTabs();
	os << "[ERROR] ";
	os << message << endl;
	SetConsoleTextAttribute(hConsole, 7);
}
