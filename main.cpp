#include <QCoreApplication>

#include "src/WaveNet.h"
#include "src/WaveNetLoader.h"
#include <iostream>

#include "src/audioprocessor.h"

#include <src/Eq4Band.h>

std::vector<float> processWaveNet(WaveNet& model, const std::vector<float>& inputData, int sampleRate, int numChannels) {
    // Ses verisini kanal sayısına göre yeniden şekillendirme
    // inputData, örneğin mono ses verisi olduğunda tek bir kanal olarak düşünülebilir.
    // Eğer stereo ise, bunu uygun şekilde işlemek için dönüştürme yapılabilir.

    // Ses verisini kanal başına (numChannels) ayırarak uygun formatta hazırlıyoruz.
    std::cout << numChannels;
    std::vector<AudioBuffer> inputChannels(numChannels);
    for (int ch = 0; ch < numChannels; ++ch) {
        // Her bir kanal için veriyi ayırıyoruz
        inputChannels[ch].resize(inputData.size() / numChannels);
        for (int i = 0; i < inputData.size() / numChannels; ++i) {
            inputChannels[ch][i] = inputData[ch * inputData.size() / numChannels + i];
        }
    }

    // Çıkış verisi için uygun bir bellek ayırıyoruz
    std::vector<AudioBuffer> outputChannels(numChannels);
    for (int ch = 0; ch < numChannels; ++ch) {
        outputChannels[ch].resize(inputData.size() / numChannels);
    }

    // Modeli işliyoruz: inputChannels ve outputChannels'u modele besliyoruz
    model.process(inputChannels, outputChannels, inputData.size() / numChannels);

    // Çıkış verilerini tek bir vektörde birleştirelim
    std::vector<float> outputData(inputData.size());
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < outputChannels[ch].size(); ++i) {
            outputData[ch * outputChannels[ch].size() + i] = outputChannels[ch][i]* 16;
        }
    }


    return outputData;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    try {
        // 1. Wav dosyasını oku
        int sampleRate, numChannels;
        std::vector<float> inputData_raw = AudioProcessor::readWavFile("input.wav", sampleRate, numChannels);

        std::vector<float> inputData = std::vector<float>(inputData_raw.begin()+ (inputData_raw.size()*2)/60, inputData_raw.begin() + (inputData_raw.size()*30)/60);

        // 2. WaveNet modeli yükle
        WaveNetLoader loader("config.json");
        WaveNet model(loader.inputChannels, loader.outputChannels, loader.numChannels,
                      loader.filterWidth, loader.activation, loader.dilations);
        loader.loadVariables(model);
        Eq4Band eq4band;
        eq4band.setParameters(8, 8, 8, 8);

        // 3. Modeli çalıştır
        auto outputData = processWaveNet(model, inputData, sampleRate, numChannels);

        std::vector<float> outputData_eq(outputData.size());
        eq4band.process(outputData.data(), outputData_eq.data(), outputData.size(), numChannels, sampleRate);

        // 4. Çıktıyı WAV dosyasına kaydet
        AudioProcessor::writeWavFile("output.wav", outputData, sampleRate, numChannels);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    std::cout << "end "<< std::endl;

    return a.exec();
}
