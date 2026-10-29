#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

struct WavFormat {
    uint16_t audio_format = 1;
    uint16_t channels = 1;
    uint32_t sample_rate = 44100;
    uint32_t byte_rate = 88200;
    uint16_t block_align = 2;
    uint16_t bits_per_sample = 16;
};

class Waveform {
public:
    Waveform() = default;
    explicit Waveform(std::vector<int16_t> samples);
    Waveform(WavFormat format, std::vector<int16_t> samples);

    const WavFormat& getFormat() const;
    const std::vector<int16_t>& getSamples() const;
    std::vector<int16_t>& getSamples();
    size_t getSampleCount() const;
    bool empty() const;
    void setSamples(std::vector<int16_t> samples);
    void setFormat(WavFormat format);

private:
    WavFormat format_;
    std::vector<int16_t> samples_;
};

int16_t clampToSample(double value);
size_t secondsToSamples(double seconds, uint32_t sample_rate);
size_t millisecondsToSamples(double milliseconds, uint32_t sample_rate);
