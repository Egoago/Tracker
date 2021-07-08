#include "ConfigParser.h"
#include <iostream>
#include <fstream>
#include <string>

ConfigParser::ConfigParser(const char* fileName) : fileName(fileName)
{
    std::ifstream file(Folder + std::string(fileName));
    std::string line;
    while (std::getline(file, line)) {
        size_t del = line.find(delimeter);
        std::string key = line.substr(0, del);
        std::string value = line.substr(del+1);
        map[key] = value;
    }
    file.close();
}

const char* ConfigParser::getEntry(const char* entryName)
{
    return map.at(entryName).c_str();
}

void ConfigParser::setEntry(const char* entryName, const char* value)
{
    map[entryName] = value;
    save();
}

void ConfigParser::save()
{
    std::ofstream file(Folder + std::string(fileName));
    for (auto const& entry : map)
        file << entry.first << delimeter << entry.second << std::endl;
    file.close();
}

ConfigParser::~ConfigParser()
{
    save();
}
