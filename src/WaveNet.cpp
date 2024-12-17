/*
  ==============================================================================

    WaveNet.cpp
    Created: 14 Jan 2019 5:19:01pm
    Author:  Damskägg Eero-Pekka

  ==============================================================================
*/

#include "WaveNet.h"


WaveNet::WaveNet(int inputChannels, int outputChannels, int convolutionChannels,
                 int filterWidth, std::string activation, std::vector<int> dilations) :
    convStack(convolutionChannels, filterWidth, dilations, activation),
    inputLayer(inputChannels, convolutionChannels, 1),
    outputLayer(convolutionChannels * dilations.size(), outputChannels, 1),
    inputChannels(inputChannels),
    outputChannels(outputChannels),
    filterWidth(filterWidth),
    skipChannels(convolutionChannels * (int)dilations.size()),
    convolutionChannels(convolutionChannels),
    memoryChannels(Activations::isGated(activation) ? convolutionChannels * 2 : convolutionChannels),
    activation(activation),
    dilations(dilations)
{
}

void WaveNet::readDilations(const var &config)
{
    std::vector<int> newDilations;

    if (auto dilationsArray = std::get_if<std::vector<int>>(&config))
    {
        // Eğer doğrudan bir vektör gelmişse
        newDilations = *dilationsArray;
    }
    else if (auto dilationsStr = std::get_if<std::string>(&config))
    {
        // Eğer string olarak gelmişse (örneğin "1,2,4,8")
        std::stringstream ss(*dilationsStr);
        int value;
        while (ss >> value)
        {
            newDilations.push_back(value);
            if (ss.peek() == ',')
                ss.ignore();
        }
    }

    dilations = newDilations;
}


void WaveNet::prepareToPlay(int newSamplesPerBlock)
{
    samplesPerBlock = newSamplesPerBlock;
    convData.resize(samplesPerBlock * memoryChannels, 0.0f);
    skipData.resize(samplesPerBlock * skipChannels, 0.0f);
    convStack.prepareToPlay(samplesPerBlock);
}

void WaveNet::copyInputData(const std::vector<AudioBuffer>& inputData, int numSamples)
{
    for (int ch = 0; ch < inputChannels; ++ch)
    {
        int start_idx = idx(ch, 0, numSamples);
        const AudioBuffer& chData = inputData[ch];
        for (int i = 0; i < numSamples; ++i)
            convData[start_idx + i] = chData[i];
    }
}

void WaveNet::copyOutputData(std::vector<AudioBuffer>& outputData, int numSamples)
{
    for (int ch = 0; ch < outputChannels; ++ch)
    {
        int start_idx = idx(ch, 0, numSamples);
        AudioBuffer& chData = outputData[ch];
        for (int i = 0; i < numSamples; ++i)
            chData[i] = skipData[start_idx + i];
    }
}

void WaveNet::process(std::vector<AudioBuffer> inputData, std::vector<AudioBuffer> &outputData, int numSamples)
{
    if (numSamples > samplesPerBlock)
        prepareToPlay(numSamples);

    copyInputData(inputData, numSamples);
    inputLayer.process(convData.data(), numSamples);
    convStack.process(convData.data(), skipData.data(), numSamples);
    outputLayer.process(skipData.data(), numSamples);
    copyOutputData(outputData, numSamples);
}

int WaveNet::idx(int ch, int i, int numSamples)
{
    return ch * numSamples + i;
}

void WaveNet::setWeight(std::vector<float> W, int layerIdx, std::string name)
{
    if (layerIdx < 0)
    {
        inputLayer.setWeight(W, name);
    }
    else if (layerIdx >= convStack.getNumLayers())
    {
        outputLayer.setWeight(W, name);
    }
    else
    {
        convStack.setWeight(W, layerIdx, name);
    }
}

void WaveNet::setParams(int newInputChannels, int newOutputChannels, int newConvChannels,
                        int newFilterWidth, std::string newActivation,
                        std::vector<int> newDilations, float levelAdjust_in)
{
    levelAdjust = levelAdjust_in;
    inputChannels = newInputChannels;
    outputChannels = newOutputChannels;
    activation = newActivation;
    convolutionChannels = newConvChannels;
    memoryChannels = Activations::isGated(activation) ? convolutionChannels * 2 : convolutionChannels;
    filterWidth = newFilterWidth;
    dilations = newDilations;
    skipChannels = convolutionChannels * (int)dilations.size();
    inputLayer.setParams(inputChannels, convolutionChannels, 1, 1, false, "linear");
    outputLayer.setParams(skipChannels, outputChannels, 1, 1, false, "linear");
    convStack.setParams(convolutionChannels, filterWidth, dilations, activation, true);
    prepareToPlay(samplesPerBlock);
}
