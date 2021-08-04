#include "Log.hpp"
//TODO only for windows
#include <Windows.h>

using namespace std;
using namespace tr;

void* Logger::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
unordered_map<string, Logger::TimePoint> Logger::runningProcesses;
unordered_set<string> Logger::warnings;
unordered_set<string> Logger::errors;
bool Logger::logging = true;
int Logger::defaultColor = 7;
ostream& Logger::os = std::cout;

void Logger::printTabs() {
	os << string(runningProcesses.size(), '\t');
}

void Logger::logProcess(const string& processName) {
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
	if (!logging) return;
	SetConsoleTextAttribute(hConsole, 8);
	Logger::printTabs();
	os << message;
	if (!noEndl)
		os << endl;
	SetConsoleTextAttribute(hConsole, defaultColor);
}

void Logger::warning(const string& message, bool repeat) {
	if (!logging) return;
	if (warnings.count(message) != 0) {
		if(!repeat) return;
	}
	else warnings.insert(message);
	SetConsoleTextAttribute(hConsole, 14);
	Logger::printTabs();
	os << "[WARNING] ";
	os << message << endl;
	SetConsoleTextAttribute(hConsole, defaultColor);
}

void Logger::error(const string& message) {
	if (errors.count(message) != 0) return;
	errors.insert(message);
	SetConsoleTextAttribute(hConsole, 12);
	Logger::printTabs();
	os << "[ERROR] ";
	os << message << endl;
	SetConsoleTextAttribute(hConsole, defaultColor);
}
