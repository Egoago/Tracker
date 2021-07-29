#pragma once
#include <map>
#include <string>
#include <vector>
//TODO remove #include <serializer.h>

namespace tr
{
	class ConfigParser
	{

	private:
		const std::string delimiter = "=";
		const std::string subDelimiter = ";";
		std::map<std::string, std::vector<std::string>> configuration;
		std::string fileName;
		std::vector<std::string>& operator[](const std::string& entryName);

	public:
		void load();
		void save();
		ConfigParser(const char* fileName);
		std::string ConfigParser::getEntry(const std::string& entryName);
		std::string ConfigParser::getEntry(const std::string& entryName, const std::string& defaultValue);
		void ConfigParser::setEntry(const std::string& entryName, const std::string& value);
		std::vector<std::string>& ConfigParser::getEntries(const std::string& entryName);
		std::vector<std::string>& ConfigParser::getEntries(const std::string& entryName, const std::vector<std::string> defaultValues);
		~ConfigParser();

		/*friend std::ostream& operator<<(std::ostream& out, Bits<class ConfigParser&> object)
		{
			out << bits(object.t.fileName) << bits(object.t.configuration);
			return (out);
		}
		friend std::istream& operator>>(std::istream& in, Bits<class ConfigParser&> object)
		{
			in >> bits(object.t.fileName) >> bits(object.t.configuration);
			object.t.load();
			return (in);
		}*/
	};
}

