#pragma once
#include <map>
#include <string>
#include <vector>
#include <type_traits>
#include <algorithm>
#include <iterator>

namespace tr {
	class ConfigParser { 
	public:
		typedef std::string Entry, Key;
		typedef std::vector<Entry> MultiEntry;
		typedef std::map<Key, MultiEntry> Section;

	private:
		bool loaded = false;
		std::map<Key, Section> configuration;

		void entry2Data(const Entry& entryName, std::string& data);
		void entry2Data(const Entry& entryName, bool& data);
		void entry2Data(const Entry& entryName, int& data);
		void entry2Data(const Entry& entryName, unsigned int& data);
		void entry2Data(const Entry& entryName, float& data);
		void entry2Data(const Entry& entryName, double& data);
		ConfigParser() {}
		void load();
		void save();
		bool validKey(const Key& token);
		bool validEntry(const Entry& token);
		bool validEntry(const MultiEntry& token);
	public:
		static ConfigParser& instance() {
			static ConfigParser Instance;
			if (!Instance.loaded) Instance.load();
			return Instance;
		}

		template<typename EntryType = std::string>
		EntryType getEntry(const Key& sectionName, const Key& entryName);

		template<typename EntryType = std::string>
		EntryType getEntry(const Key& sectionName, const Key& entryName, const EntryType& defaultValue);

		template<typename EntryType = std::string>
		void setEntry(const Key& sectionName, const Key& entryName, const EntryType& value);

		template<typename EntryType = std::string>
		void setEntries(const Key& sectionName, const Key& entryName, const std::vector<EntryType>& value);

		template<typename EntryType = std::string>
		std::vector<EntryType> getEntries(const Key& sectionName, const Key& entryName);

		template<typename EntryType = std::string>
		std::vector<EntryType> getEntries(const Key& sectionName, const Key& entryName, const std::vector<EntryType>& defaultValues);
		~ConfigParser();
	};

	template<typename EntryType>
	inline EntryType tr::ConfigParser::getEntry(const Key& sectionName, const Key& entryName) {
		return getEntries<EntryType>(sectionName, entryName)[0];
	}

	template<typename EntryType>
	inline EntryType ConfigParser::getEntry(const Key& sectionName, const Key& entryName, const EntryType& defaultValue) {
		return getEntries<EntryType>(sectionName, entryName, { defaultValue })[0];
	}

	template<typename EntryType>
	inline std::vector<EntryType> ConfigParser::getEntries(const Key& sectionName, const Key& entryName) {
		const MultiEntry& entries = configuration[sectionName][entryName];
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
	inline std::vector<EntryType> ConfigParser::getEntries(const Key& sectionName, const Key& entryName, const std::vector<EntryType>& defaultValues) {
		if (configuration.count(sectionName) == 0 || configuration[sectionName].count(entryName) == 0)
			setEntries(sectionName, entryName, defaultValues);
		return getEntries<EntryType>(sectionName, entryName);
	}

	template<typename EntryType>
	inline void tr::ConfigParser::setEntry(const Key& sectionName, const Key& entryName, const EntryType& value) {
		setEntries(sectionName, entryName, { value });
	}


	template<>
	inline void ConfigParser::setEntries(const Key& sectionName, const Key& entryName, const std::vector<bool>& value) {
		std::vector<std::string> newValues;
		newValues.reserve(value.size());
		std::transform(std::begin(value),
			std::end(value),
			std::back_inserter(newValues),
			[](bool entry) { return entry ? "true" : "false"; }
		);
		setEntries<std::string>(sectionName, entryName, newValues);
	}

	template<typename EntryType>
	inline void ConfigParser::setEntries(const Key& sectionName, const Key& entryName, const std::vector<EntryType>& value) {
		std::vector<std::string> strValues;
		if constexpr (std::is_same_v<EntryType, std::string>)
			strValues = value;
		else {
			strValues.reserve(value.size());
			std::transform(std::begin(value),
				std::end(value),
				std::back_inserter(strValues),
				[](EntryType entry) { return std::to_string(entry); }
			);
		}
		if (validKey(sectionName) && validEntry(entryName) && validEntry(strValues)) {
			configuration[sectionName][entryName].clear();
			configuration[sectionName][entryName] = strValues;
			save();
		}
		//setEntries<std::string>(sectionName, entryName, newValues);
	}
}

