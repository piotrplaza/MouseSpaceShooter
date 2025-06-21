#include "paramsFromFile.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace Tools
{
	ParamsFromFile::ParamsFromFile(const std::string& filePath):
		keysToValues([&]() {
			std::multimap<std::string, std::string> keysToValues;
			std::ifstream file(filePath);
			if (!file.is_open())
				std::cout << "unable to open \"" << filePath << "\"" << std::endl;

			std::string line;
			while (std::getline(file, line))
			{
				std::istringstream iss(line);
				std::string key, value;

				if (iss >> key) {
					std::getline(iss, value);
					value.erase(0, 1);
					keysToValues.emplace(std::move(key), std::move(value));
				}
			}

			std::cout << keysToValues.size() << " params loaded:" << std::endl;
			for (const auto& [key, value] : keysToValues)
				std::cout << key << " " << value << std::endl;
			std::cout << std::endl;
			return keysToValues;
		}())
	{
	}

	std::string ParamsFromFile::getValue(const std::string& key) const
	{
		auto range = keysToValues.equal_range(key);
		if (range.first == range.second)
			throw std::runtime_error("loadParams(): Key " + key + " not found");
		if (std::distance(range.first, range.second) > 1)
			throw std::runtime_error("loadParams(): Multiple values for key " + key);
		return range.first->second;
	};

	std::vector<std::string> ParamsFromFile::getValues(const std::string& key) const
	{
		auto range = keysToValues.equal_range(key);
		if (range.first == range.second)
			throw std::runtime_error("loadParams(): Key " + key + " not found");
		std::vector<std::string> values;
		values.reserve(std::distance(range.first, range.second));
		for (auto it = range.first; it != range.second; ++it)
			values.push_back(it->second);
		return values;
	};
}
