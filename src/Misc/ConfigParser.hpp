#pragma once
#include <map>
#include <string>
#include <vector>
#include <type_traits>

namespace tr {
	class ConfigParser {

	private:
		const std::string delimiter = "=";
		const std::string subDelimiter = ";";
		std::map<std::string, std::vector<std::string>> configuration;
		std::string fileName;
		template<typename EntryType = std::string>
		EntryType getEntry(const std::string& entryName);
		
		void entry2Data(const std::string& entryName, std::string& data);
		void entry2Data(const std::string& entryName, bool& data);
		void entry2Data(const std::string& entryName, int& data);
		void entry2Data(const std::string& entryName, float& data);
		void entry2Data(const std::string& entryName, double& data);

	public:
		void load();
		void save();
		ConfigParser(const char* fileName);

		template<typename EntryType = std::string>
		EntryType getEntry(const std::string& entryName, const EntryType& defaultValue);

		template<typename EntryType = std::string>
		void setEntry(const std::string& entryName, const EntryType& value);

		template<typename EntryType = std::string>
		void setEntries(const std::string& entryName, const std::vector<EntryType>& value);

		template<typename EntryType = std::string>
		std::vector<EntryType> getEntries(const std::string& entryName);

		template<typename EntryType = std::string>
		std::vector<EntryType> getEntries(const std::string& entryName, const std::vector<EntryType>& defaultValues);
		~ConfigParser();
	};

	template<typename EntryType>
	inline EntryType tr::ConfigParser::getEntry(const std::string& entryName)
	{ return getEntries<EntryType>(entryName)[0]; }

	template<typename EntryType>
	inline EntryType ConfigParser::getEntry(const std::string& entryName, const EntryType& defaultValue)
	{ return getEntries<EntryType>(entryName, { defaultValue })[0]; }

	template<typename EntryType>
	inline std::vector<EntryType> ConfigParser::getEntries(const std::string& entryName) {
		const std::vector<std::string>& entries = configuration[entryName];
		const unsigned int entriesCount = (unsigned int)entries.size();
		std::vector<EntryType> outEntries(entriesCount);
		for (unsigned int i = 0; i < entriesCount; i++) {
			EntryType dummy; //needed because vector<bool> is kinda messed up
			entry2Data(entries[i], dummy);
			outEntries[i] = dummy;
		}
		return outEntries;
	}

	template<typename EntryType>
	inline std::vector<EntryType> ConfigParser::getEntries(const std::string& entryName, const std::vector<EntryType>& defaultValues) {
		if (configuration.count(entryName) == 0) {
			setEntries(entryName, defaultValues);
			save();
		}
		return getEntries<EntryType>(entryName);
	}

	template<typename EntryType>
	inline void tr::ConfigParser::setEntry(const std::string& entryName, const EntryType& value)
	{ setEntries(entryName, {value}); }

	template<>
	inline void ConfigParser::setEntries(const std::string& entryName, const std::vector<std::string>& value)
	{ configuration[entryName] = value; }
	template<>
	inline void ConfigParser::setEntries(const std::string& entryName, const std::vector<bool>& value) {
		configuration[entryName].clear();
		for (const auto entry : value)
			configuration[entryName].push_back( entry ? "true" : "false");
	}
	template<typename EntryType>
	inline void ConfigParser::setEntries(const std::string& entryName, const std::vector<EntryType>& value) {
		configuration[entryName].clear();
		for (const EntryType entry : value)
			configuration[entryName].push_back(std::to_string(entry));
	}
}

