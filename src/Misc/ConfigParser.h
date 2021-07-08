#pragma once
#include <map>
#include <string>

class ConfigParser
{
private:
	const char* Folder = "Config/";
	const static char delimeter = '=';
	std::map<std::string, std::string> map;
	std::string fileName;
	void save();
public:
	ConfigParser(const char* fileName);
	const char* getEntry(const char* entryName);
	void setEntry(const char* entryName, const char* value);
	~ConfigParser();
};

