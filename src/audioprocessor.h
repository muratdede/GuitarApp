#ifndef AUDIOPROCESSOR_H
#define AUDIOPROCESSOR_H

#include <QFile>
#include <QByteArray>
#include <iostream>
#include <vector>

struct WavHeader {
    char riff[4];  // "RIFF"
    uint32_t size;
    char wave[4];  // "WAVE"
    char fmt[4];   // "fmt "
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];  // "data"
    uint32_t dataSize;
};

class AudioProcessor
{
public:
    static std::vector<float> readWavFile(const QString& filename, int& sampleRate, int& numChannels) {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            std::cerr << "Error opening WAV file: " << filename.toStdString() << std::endl;
            return {};
        }

        // Read the header
        WavHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

        // Check for valid WAV file
        if (strncmp(header.riff, "RIFF", 4) != 0 || strncmp(header.wave, "WAVE", 4) != 0) {
            std::cerr << "Invalid WAV file." << std::endl;
            return {};
        }

        // Extract format information
        sampleRate = header.sampleRate;
        numChannels = header.numChannels;

        // Read the audio data
        QByteArray audioData = file.read(header.dataSize);
        file.close();

        // Convert audio data to float values (assuming 16-bit PCM)
        std::vector<float> audioSamples;
        for (int i = 0; i < audioData.size(); i += 2) {
            int16_t sample = (static_cast<uint8_t>(audioData[i + 1]) << 8) | static_cast<uint8_t>(audioData[i]);
            audioSamples.push_back(static_cast<float>(sample) / 32768.0f);  // Normalize to [-1.0, 1.0]
        }

        return audioSamples;
    }

    static void writeWavFile(const QString& filename, const std::vector<float>& data, int sampleRate, int numChannels) {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            std::cerr << "Error opening file for writing: " << filename.toStdString() << std::endl;
            return;
        }

        // Create the header
        WavHeader header;
        memcpy(header.riff, "RIFF", 4);
        memcpy(header.wave, "WAVE", 4);
        memcpy(header.fmt, "fmt ", 4);
        header.fmtSize = 16;
        header.audioFormat = 1;  // PCM format
        header.numChannels = numChannels;
        header.sampleRate = sampleRate;
        header.byteRate = sampleRate * numChannels * 2;  // 16-bit samples
        header.blockAlign = numChannels * 2;
        header.bitsPerSample = 16;
        memcpy(header.data, "data", 4);
        header.dataSize = data.size() * 2;  // 16-bit samples, 2 bytes each

        // Write the header
        file.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));

        // Write the audio data
        QByteArray audioData;
        for (float sample : data) {
            int16_t intSample = static_cast<int16_t>(sample * 32767);  // Convert back to int16
            audioData.append(static_cast<char>(intSample & 0xFF));
            audioData.append(static_cast<char>((intSample >> 8) & 0xFF));
        }

        file.write(audioData);
        file.close();
    }

private:
    AudioProcessor() = delete;
};

#endif // AUDIOPROCESSOR_H
