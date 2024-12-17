/*
  ==============================================================================

    WaveNetLoader.cpp
    Created: 3 Feb 2019 8:55:31pm
    Author:  Eero-Pekka Damskägg

    Modified: JUCE bağımlılığı kaldırıldı

  ==============================================================================
*/

#include "WaveNetLoader.h"

WaveNetLoader::WaveNetLoader(const std::string& jsonFilePath)
{
    // JSON dosyasını okuma
    std::ifstream file(jsonFilePath);
    if (!file.is_open())
        throw std::runtime_error("Unable to open JSON file: " + jsonFilePath);

    file >> config;

    if (config.contains("level_adjust"))
        levelAdjust = config["level_adjust"].get<float>();

    numChannels = config["residual_channels"].get<int>();
    inputChannels = config["input_channels"].get<int>();
    outputChannels = config["output_channels"].get<int>();
    filterWidth = config["filter_width"].get<int>();
    activation = config["activation"].get<std::string>();
    dilations = readDilations();
}

WaveNetLoader::WaveNetLoader(const std::string& jsonFilePath, const std::string& configFilePath)
{
    // Konfigürasyon dosyasını okuma
    std::ifstream file(configFilePath);
    if (!file.is_open())
        throw std::runtime_error("Unable to open config file: " + configFilePath);

    file >> config;

    if (config.contains("level_adjust"))
        levelAdjust = config["level_adjust"].get<float>();

    numChannels = config["residual_channels"].get<int>();
    inputChannels = config["input_channels"].get<int>();
    outputChannels = config["output_channels"].get<int>();
    filterWidth = config["filter_width"].get<int>();
    activation = config["activation"].get<std::string>();
    dilations = readDilations();
}

std::vector<int> WaveNetLoader::readDilations()
{
    std::vector<int> newDilations;

    if (config.contains("dilations") && config["dilations"].is_array())
    {
        for (const auto& dil : config["dilations"])
            newDilations.push_back(dil.get<int>());
    }
    return newDilations;
}

void WaveNetLoader::loadVariables(WaveNet &model)
{
    if (config.contains("variables") && config["variables"].is_array())
    {
        for (const auto& variable : config["variables"])
        {
            int layerIdx = variable["layer_idx"].get<int>();
            std::string name = variable["name"].get<std::string>();
            std::vector<float> data;

            if (variable.contains("data") && variable["data"].is_array())
            {
                for (const auto& value : variable["data"])
                    data.push_back(value.get<float>());
            }

            model.setWeight(data, layerIdx, name);
        }
    }
}
