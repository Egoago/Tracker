#include "ConfigParser.hpp"
#include <iostream>
#include <fstream>
#include "Links.hpp"

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

ConfigParser::ConfigParser(const char* fileName) : fileName(fileName)
{ load(); }

void tr::ConfigParser::entry2Data(const std::string& entryName, std::string& data)
{ data = entryName; }

void tr::ConfigParser::entry2Data(const std::string& entryName, bool& data)
{ data = entryName.compare("true") == 0; }

void tr::ConfigParser::entry2Data(const std::string& entryName, int& data)
{ data = stoi(entryName); }

void tr::ConfigParser::entry2Data(const std::string& entryName, unsigned int& data)
{ data = (unsigned int)stoi(entryName); }

void tr::ConfigParser::entry2Data(const std::string& entryName, float& data)
{ data = stof(entryName); }

void tr::ConfigParser::entry2Data(const std::string& entryName, double& data)
{ data = stod(entryName); }

void ConfigParser::load() {
    ifstream file(CONFIG_FOLDER + string(fileName));
    string line;
    while (getline(file, line)) {
        vector<string> tokens = split(line, delimiter);
        if (tokens.size() > 1)
            configuration[tokens[0]] = split(tokens[1], subDelimiter);
        else if (tokens.size() == 1)
            configuration[tokens[0]] = vector<string>();
    }
    file.close();
}

void ConfigParser::save() {
    ofstream file(CONFIG_FOLDER + string(fileName) + string(CONFIG_EXTENSION));
    for (auto const& entry : configuration) {
        bool first = true;
        file << entry.first << delimiter;
        for (auto const& value : entry.second)
            if (first) {
                file << value;
                first = false;
            }
            else file << ';' << value;
        file << endl;
    }
    file.close();
}

ConfigParser::~ConfigParser()
{ save(); }
