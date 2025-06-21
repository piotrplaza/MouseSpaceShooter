#pragma once

#include <tools/utility.hpp>

#include <string>
#include <vector>
#include <map>

namespace Tools
{
	class ParamsFromFile
	{
	public:
		ParamsFromFile(const std::string& filePath);
		std::string getValue(const std::string& key) const;
		std::vector<std::string> getValues(const std::string& key) const;
		void loadParam(auto& param, const std::string& key, bool mandatory) const
		{
			using ParamType = std::remove_reference_t<decltype(param)>;

			try
			{
				if constexpr (std::is_same_v<ParamType, int>)
					param = Tools::Stoi(getValue(key));
				else if constexpr (std::is_same_v<ParamType, float>)
					param = Tools::Stof(getValue(key));
				else if constexpr (std::is_same_v<ParamType, bool>)
					param = getValue(key) == "true";
				else if constexpr (std::is_same_v<ParamType, std::string>)
					param = getValue(key);
				else if constexpr (std::is_same_v<ParamType, glm::vec2>)
				{
					const auto value = getValue(key);
					const auto spacePos = value.find(' ');
					param = { Tools::Stof(value.substr(0, spacePos)), Tools::Stof(value.substr(spacePos + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::ivec2>)
				{
					const auto value = getValue(key);
					const auto spacePos = value.find(' ');
					param = { Tools::Stoi(value.substr(0, spacePos)), Tools::Stoi(value.substr(spacePos + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::vec3>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					param = { Tools::Stof(value.substr(0, spacePos1)), Tools::Stof(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)), Tools::Stof(value.substr(spacePos2 + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::ivec3>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					param = { Tools::Stoi(value.substr(0, spacePos1)), Tools::Stoi(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)), Tools::Stoi(value.substr(spacePos2 + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::vec4>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					const auto spacePos3 = value.find(' ', spacePos2 + 1);
					param = { Tools::Stof(value.substr(0, spacePos1)), Tools::Stof(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)),
						Tools::Stof(value.substr(spacePos2 + 1, spacePos3 - spacePos2 - 1)), Tools::Stof(value.substr(spacePos3 + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, glm::ivec4>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					const auto spacePos3 = value.find(' ', spacePos2 + 1);
					param = { Tools::Stoi(value.substr(0, spacePos1)), Tools::Stoi(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)),
						Tools::Stoi(value.substr(spacePos2 + 1, spacePos3 - spacePos2 - 1)), Tools::Stoi(value.substr(spacePos3 + 1)) };
				}
				else if constexpr (std::is_same_v<ParamType, std::array<std::string, 4>>)
				{
					const auto value = getValue(key);
					const auto spacePos1 = value.find(' ');
					const auto spacePos2 = value.find(' ', spacePos1 + 1);
					const auto spacePos3 = value.find(' ', spacePos2 + 1);
					param = { value.substr(0, spacePos1), value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1),
						value.substr(spacePos2 + 1, spacePos3 - spacePos2 - 1), value.substr(spacePos3 + 1) };
				}
				else if constexpr (std::is_same_v<ParamType, std::vector<std::array<float, 5>>>)
				{
					const auto values = getValues(key);
					param.reserve(values.size());
					for (const auto& value : values)
					{
						const auto spacePos1 = value.find(' ');
						const auto spacePos2 = value.find(' ', spacePos1 + 1);
						const auto spacePos3 = value.find(' ', spacePos2 + 1);
						const auto spacePos4 = value.find(' ', spacePos3 + 1);
						param.push_back({
							Tools::Stof(value.substr(0, spacePos1)),
							Tools::Stof(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)),
							Tools::Stof(value.substr(spacePos2 + 1, spacePos3 - spacePos2 - 1)),
							Tools::Stof(value.substr(spacePos3 + 1, spacePos4 - spacePos3 - 1)),
							Tools::Stof(value.substr(spacePos4 + 1)) });
					}
				}
				else if constexpr (std::is_same_v<ParamType, std::vector<glm::vec3>>)
				{
					const auto values = getValues(key);
					param.reserve(values.size());
					for (const auto& value : values)
					{
						const auto spacePos1 = value.find(' ');
						const auto spacePos2 = value.find(' ', spacePos1 + 1);
						param.push_back({
							Tools::Stof(value.substr(0, spacePos1)),
							Tools::Stof(value.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1)),
							Tools::Stof(value.substr(spacePos2 + 1)) });
					}
				}
				else
					throw std::runtime_error("Unsupported value for key " + key);
			}
			catch (const std::runtime_error& e)
			{
				if (mandatory || e.what() != "loadParams(): Key " + key + " not found")
					throw e;
			}
			catch (...)
			{
				throw;
			}
		}

	private:
		const std::multimap<std::string, std::string> keysToValues;
	};
}
