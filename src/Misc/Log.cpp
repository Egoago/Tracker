#include "Log.h"
//TODO only for windows
//#include <consoleapi2.h>
//#include <WinBase.h>

using namespace std;

const HANDLE Logger::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

unordered_set<string> Logger::runningProcesses;

bool Logger::logging = true;

void Logger::printTabs() {
	cout << string(runningProcesses.size(), '\t');
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
		//cout << processName << endl;
	}
}

void Logger::log(const string& message, bool noEndl) {
	if (!logging) return;
	Logger::printTabs();
	cout << message;
	if (!noEndl)
		cout << endl;
}

void Logger::warning(const string& message) {
	if (!logging) return;
	SetConsoleTextAttribute(hConsole, 14);
	Logger::printTabs();
	cout << "[WARNING] ";
	cout << message << endl;
	SetConsoleTextAttribute(hConsole, 7);
}

void Logger::error(const string& message) {
	SetConsoleTextAttribute(hConsole, 12);
	Logger::printTabs();
	cout << "[ERROR] ";
	cout << message << endl;
	SetConsoleTextAttribute(hConsole, 7);
}
