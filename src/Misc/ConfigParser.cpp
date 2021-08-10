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
{
    load();
}

string ConfigParser::getEntry(const string& entryName) {
    return (*this)[entryName][0];
}

string ConfigParser::getEntry(const string& entryName, const string& defaultValue)
{
    if (configuration.count(entryName) == 0) {
        configuration[entryName] = vector<string>{ defaultValue };
        save();
    }
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
    if (configuration.count(entryName) == 0)
        configuration[entryName] = defaultValues;
    return getEntries(entryName);
}

vector<string>& ConfigParser::operator[](const string& entryName)
{
    return configuration[entryName];
}

void ConfigParser::load()
{
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

void ConfigParser::save()
{
    ofstream file(CONFIG_FOLDER + string(fileName));
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
{
    save();
}
