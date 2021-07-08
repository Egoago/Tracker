#pragma once
#include <map>
#include <string>
#include <vector>

class ConfigParser
{

private:
	const std::string delimiter = "=";
	const std::string subDelimiter = ";";
	std::map<std::string, std::vector<std::string>> map;
	std::string fileName;
	std::vector<std::string>& operator[](const std::string& entryName);

public:
	void save();
	ConfigParser(const char* fileName);
	std::string ConfigParser::getEntry(const std::string& entryName);
	std::string ConfigParser::getEntry(const std::string& entryName, const std::string& defaultValue);
	void ConfigParser::setEntry(const std::string& entryName, const std::string& value);
	std::vector<std::string>& ConfigParser::getEntries(const std::string& entryName);
	std::vector<std::string>& ConfigParser::getEntries(const std::string& entryName, const std::vector<std::string> defaultValues);
	~ConfigParser();
};

