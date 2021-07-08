#include "ConfigParser.h"
#include <iostream>
#include <fstream>
#include <string>
#include "Links.h"

using namespace std;

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
{
    ifstream file(CONFIG_FOLDER + string(fileName));
    string line;
    while (getline(file, line)) {
        vector<string> tokens = split(line, delimiter);
        if (tokens.size() > 1)
            map[tokens[0]] = split(tokens[1], subDelimiter);
        else if (tokens.size() == 1)
            map[tokens[0]] = vector<string>();
    }
    file.close();
}

string ConfigParser::getEntry(const string& entryName) {
    return (*this)[entryName][0];
}

string ConfigParser::getEntry(const string& entryName, const string& defaultValue)
{
    if (map.count(entryName) == 0)
        map[entryName] = vector<string>{ defaultValue };
    return getEntry(entryName);
}

void ConfigParser::setEntry(const string& entryName, const string& value)
{
    (*this)[entryName] = vector<string>{ value };
    save();
}

vector<string>& ConfigParser::getEntries(const string& entryName)
{
    return (*this)[entryName];
}

vector<string>& ConfigParser::getEntries(const string& entryName, const vector<string> defaultValues)
{
    if (map.count(entryName) == 0)
        map[entryName] = defaultValues;
    return getEntries(entryName);
}

vector<string>& ConfigParser::operator[](const string& entryName)
{
    return map[entryName];
}

void ConfigParser::save()
{
    ofstream file(CONFIG_FOLDER + string(fileName));
    for (auto const& entry : map) {
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
{
    save();
}
