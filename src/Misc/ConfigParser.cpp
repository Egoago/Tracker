#include "ConfigParser.hpp"
#include <iostream>
#include <fstream>
#include "Constants.hpp"
#include "Log.hpp"

using namespace std;
using namespace tr;

vector<string> split(string str, string delimiter) {
    vector<string> out;
    size_t pos = 0;
    while ((pos = str.find(delimiter)) != string::npos) {
        string l = str.substr(0, pos);
        if (l.length() > 0)
            out.push_back(l);
        str.erase(0, pos + delimiter.length());
    }
    if (str.length() > 0)
        out.push_back(str);
    return out;
}

string trim(const string& str) {
    const size_t first = str.find_first_not_of(" \n\t");
    if (first == string::npos) return "";
    const auto last = str.find_last_not_of(" \n\t");
    return str.substr(first, 1 + last - first);
}

void tr::ConfigParser::entry2Data(const Entry& entryName, std::string& data) {
    data = entryName;
}

void tr::ConfigParser::entry2Data(const Entry& entryName, bool& data) {
    data = entryName.compare("true") == 0;
}

void tr::ConfigParser::entry2Data(const Entry& entryName, int& data) {
    data = stoi(entryName);
}

void tr::ConfigParser::entry2Data(const Entry& entryName, unsigned int& data) {
    data = (unsigned int)stoi(entryName);
}

void tr::ConfigParser::entry2Data(const Entry& entryName, float& data) {
    data = stof(entryName);
}

void tr::ConfigParser::entry2Data(const Entry& entryName, double& data) {
    data = stod(entryName);
}

void ConfigParser::load() {
    ifstream file(CONFIG_FILE);
    string line;
    configuration.clear();
    Key sectionKey = "";
    while (getline(file, line)) {
        vector<string> tokens = split(trim(line), CONFIG_DELIMITER);
        if (tokens.size() == 2) {
            if (tokens[0].compare(CONFIG_SECTION_LABEL) == 0) sectionKey = tokens[1];
            else configuration[sectionKey][tokens[0]] = split(tokens[1], CONFIG_SUB_DELIMITER);
        }
        else if (tokens.size() == 1 && tokens[0].size() > 0) sectionKey = tokens[0];
        else if (tokens.size() > 2)
            Logger::warning("Unable to read config entry: [" + line + "]");
    }
    file.close();
}

void ConfigParser::save() {
    ofstream file(CONFIG_FILE);
    for (auto const& section : configuration) {
        file << CONFIG_SECTION_LABEL
             << CONFIG_DELIMITER
             << section.first
             << endl;
        for (auto const& entry : section.second) {
            bool first = true;
            file << CONFIG_TABULATOR << entry.first << CONFIG_DELIMITER;
            for (auto const& value : entry.second)
                if (first) {
                    file << value;
                    first = false;
                }
                else file << CONFIG_SUB_DELIMITER << value;
            file << endl;
        }
        file << endl;
    }
    file.close();
}

bool tr::ConfigParser::validKey(const Key& key) {
    if (key.find(CONFIG_DELIMITER) != std::string::npos) {
        Logger::warning("Key can not contain delimiter character: [" + CONFIG_DELIMITER + "]");
        return false;
    }
    return true;
}

bool tr::ConfigParser::validEntry(const Entry& entry) {
    if (entry.find(CONFIG_DELIMITER) != std::string::npos ||
        entry.find(CONFIG_SUB_DELIMITER) != std::string::npos) {
        Logger::warning("Entry can not contain delimiter characters: [" + CONFIG_DELIMITER + "], [" + CONFIG_SUB_DELIMITER + "]");
        return false;
    }
    return true;
}

bool tr::ConfigParser::validEntry(const MultiEntry& multiEntry) {
    for (const Entry& entry : multiEntry)
        if (!validEntry(entry)) return false;
    return true;
}

bool tr::ConfigParser::hasEntry(const Key& sectionName, const Key& entryName) {
    if (!validKey(sectionName)) Logger::error("Section name invalid: " + sectionName);
    if (!validKey(entryName)) Logger::error("Entry name invalid: " + sectionName);
    return (configuration.count(sectionName) > 0 && configuration[sectionName].count(entryName) > 0);
}

ConfigParser::~ConfigParser() { save(); }
