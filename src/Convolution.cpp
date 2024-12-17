/*
  ==============================================================================

    Convolution.cpp
    Created: 3 Jan 2019 10:58:34am
    Author:  Damskägg Eero-Pekka

  ==============================================================================
*/

#include "Convolution.h"
#include <assert.h>

Convolution::Convolution(size_t inputChannels, size_t outputChannels, int filterWidth, int dilation) :
    bias(outputChannels),
    outVec(outputChannels),
    pos(0),
    dilation(dilation),
    inputChannels(inputChannels),
    outputChannels(outputChannels),
    filterWidth(filterWidth)
{
    resetFifo();
    resetKernel();
}

void Convolution::resetKernel()
{
    kernel.clear();
    kernel.reserve(filterWidth);
    for (int i = 0; i < filterWidth; ++i)
    {
        // Initialize the kernel as a 2D vector (inputChannels x outputChannels)
        std::vector<std::vector<float>> matrix(inputChannels, std::vector<float>(outputChannels, 0.0f));
        kernel.push_back(matrix);
    }
    bias = std::vector<float>(outputChannels, 0.0f);  // Initialize bias vector with zeros
}

void Convolution::resetFifo()
{
    memory.clear();
    memory.reserve(getFilterOrder());
    for (int i = 0; i < getFilterOrder(); ++i)
    {
        memory.push_back(std::vector<float>(inputChannels, 0.0f));  // Initialize memory with zeros
    }
    pos = 0;
}

void Convolution::setParams(size_t newInputChannels, size_t newOutputChannels,
                            int newFilterWidth, int newDilation)
{
    inputChannels = newInputChannels;
    outputChannels = newOutputChannels;
    filterWidth = newFilterWidth;
    dilation = newDilation;
    outVec = std::vector<float>(outputChannels, 0.0f);  // Initialize outVec with zeros
    resetFifo();
    resetKernel();
}

int Convolution::getFilterOrder() const
{
    return (filterWidth - 1) * dilation + 1;
}

void Convolution::process(float* data, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        processSingleSample(data, i, numSamples);
    }
}

void Convolution::processSingleSample(float* data, int i, int numSamples)
{
    if (memory.size() != getFilterOrder())
        resetFifo();

    auto fifo = memory.begin();
    for (int ch = 0; ch < inputChannels; ++ch)
        (*(fifo + pos))[ch] = data[idx(ch, i, numSamples)];

    std::fill(outVec.begin(), outVec.end(), 0.0f);  // Set outVec to zero

    for (int j = 0; j < kernel.size(); ++j)
    {
        int readPos = mod((pos - j * dilation), getFilterOrder());
        for (int ch_out = 0; ch_out < outputChannels; ++ch_out)
        {
            for (int ch_in = 0; ch_in < inputChannels; ++ch_in)
            {
                outVec[ch_out] += (*(fifo + readPos))[ch_in] * kernel[j][ch_in][ch_out];
            }
        }
    }

    for (int ch = 0; ch < outputChannels; ++ch)
        outVec[ch] += bias[ch];

    for (int ch = 0; ch < outputChannels; ++ch)
        data[idx(ch, i, numSamples)] = outVec[ch];

    pos = mod(pos + 1, getFilterOrder());
}

int Convolution::mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

int Convolution::idx(int ch, int i, int numSamples)
{
    return ch * numSamples + i;
}

void Convolution::setWeight(std::vector<float> W, std::string name)
{
    if (name == "W")
        setKernel(W);
    else if (name == "b")
        setBias(W);
}

void Convolution::setKernel(std::vector<float> W)
{
    assert(W.size() == inputChannels * outputChannels * filterWidth);
    size_t i = 0;
    for (size_t k = 0; k < filterWidth; ++k)
        for (size_t row = 0; row < inputChannels; ++row)
            for (size_t col = 0; col < outputChannels; ++col)
            {
                kernel[filterWidth - 1 - k][row][col] = W[i];
                i += 1;
            }
}

void Convolution::setBias(std::vector<float> W)
{
    assert(W.size() == outputChannels);
    for (size_t i = 0; i < outputChannels; ++i)
        bias[i] = W[i];
}
