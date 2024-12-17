/*
  ==============================================================================

    WaveNetLoader.h
    Created: 3 Feb 2019 8:55:31pm
    Author:  Eero-Pekka Damskägg

    Modified: JUCE bağımlılığı kaldırıldı

  ==============================================================================
*/

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "WaveNet.h"
#include "json.hpp"  // nlohmann/json kütüphanesi

using json = nlohmann::json;

class WaveNetLoader
{
public:
    WaveNetLoader(const std::string& jsonFilePath);
    WaveNetLoader(const std::string& jsonFilePath, const std::string& configFilePath);

    float levelAdjust = 0.0f;
    int numChannels = 0;
    int inputChannels = 0;
    int outputChannels = 0;
    int filterWidth = 0;
    std::vector<int> dilations;
    std::string activation;

    void loadVariables(WaveNet &model);

private:
    std::vector<int> readDilations();
    json config;
};
