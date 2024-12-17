/*
  ==============================================================================

    Convolution.h
    Created: 3 Jan 2019 10:58:34am
    Author:  Damskägg Eero-Pekka

  ==============================================================================
*/

#pragma once

#include <vector>
#include <string>

class Convolution
{
public:
    Convolution(size_t inputChannels, size_t outputChannels, int filterWidth, int dilation = 1);
    int getFilterOrder() const;
    void process(float* data, int numSamples);
    void setParams(size_t inputChannels, size_t outputChannels, int filterWidth, int dilation);
    size_t getNumInputChannels() { return inputChannels; }
    size_t getNumOutputChannels() { return outputChannels; }
    void setWeight(std::vector<float> W, std::string name);

private:
    std::vector<std::vector<std::vector<float>>> kernel; // Replaced Eigen::MatrixXf with std::vector
    std::vector<float> bias;  // Replaced Eigen::RowVectorXf with std::vector<float>
    std::vector<std::vector<float>> memory;  // Replaced Eigen::RowVectorXf with std::vector<float>
    std::vector<float> outVec;  // Replaced Eigen::RowVectorXf with std::vector<float>
    int pos;
    int dilation;
    size_t inputChannels;
    size_t outputChannels;
    int filterWidth;

    void resetFifo();
    void resetKernel();
    void processSingleSample(float* data, int i, int numSamples);

    int mod(int a, int b);
    int idx(int ch, int i, int numSamples);

    void setKernel(std::vector<float> W);
    void setBias(std::vector<float> W);
};
