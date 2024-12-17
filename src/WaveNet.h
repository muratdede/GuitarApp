/*
  ==============================================================================

    WaveNet.h
    Created: 14 Jan 2019 5:19:01pm
    Author:  Damskägg Eero-Pekka

  ==============================================================================
*/

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <variant>
#include "Activations.h"
#include "ConvolutionStack.h"

using AudioBuffer = std::vector<float>;
using var = std::variant<std::vector<int>, std::string>;

class WaveNet
{
public:
    WaveNet(int inputChannels, int outputChannels, int convolutionChannels,
            int filterWidth, std::string activation, std::vector<int> dilations);
    void prepareToPlay(int newSamplesPerBlock);
    void process(std::vector<AudioBuffer> inputData, std::vector<AudioBuffer> &outputData, int numSamples);
    void setWeight(std::vector<float> W, int layerIdx, std::string name);
    void setParams(int newInputChannels, int newOutputChannels, int newConvChannels,
                   int newFilterWidth, std::string newActivation,
                   std::vector<int> newDilations, float levelAdjust_in);
    float levelAdjust = 0.0;

private:
    ConvolutionStack convStack;
    ConvolutionLayer inputLayer;
    ConvolutionLayer outputLayer;
    int inputChannels;
    int outputChannels;
    int filterWidth;
    int skipChannels;
    int convolutionChannels;
    int memoryChannels;
    std::string activation;
    std::vector<int> dilations;
    int samplesPerBlock = 0;
    AudioBuffer convData;
    AudioBuffer skipData;

    int idx(int ch, int i, int numSamples);
    void readDilations(var config);
    void readDilations(const var &config);
    void copyInputData(const std::vector<AudioBuffer>& inputData, int numSamples);
    void copyOutputData(std::vector<AudioBuffer>& outputData, int numSamples);
};
