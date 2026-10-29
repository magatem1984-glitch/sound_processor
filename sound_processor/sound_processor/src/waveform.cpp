#include "sound_processor/waveform.hpp"

#include "sound_processor/error.hpp"

#include <cmath>
#include <limits>
#include <utility>

Waveform::Waveform(std::vector<int16_t> samples) : samples_(std::move(samples)) {
}

Waveform::Waveform(WavFormat format, std::vector<int16_t> samples)
    : format_(format), samples_(std::move(samples)) {
}

const WavFormat& Waveform::getFormat() const {
    return format_;
}

const std::vector<int16_t>& Waveform::getSamples() const {
    return samples_;
}

std::vector<int16_t>& Waveform::getSamples() {
    return samples_;
}

size_t Waveform::getSampleCount() const {
    return samples_.size();
}

bool Waveform::empty() const {
    return samples_.empty();
}

void Waveform::setSamples(std::vector<int16_t> samples) {
    samples_ = std::move(samples);
}

void Waveform::setFormat(WavFormat format) {
    format_ = format;
}

int16_t clampToSample(double value) {
    const double minimum = static_cast<double>(std::numeric_limits<int16_t>::min());
    const double maximum = static_cast<double>(std::numeric_limits<int16_t>::max());
    if (value < minimum) {
        return std::numeric_limits<int16_t>::min();
    }
    if (value > maximum) {
        return std::numeric_limits<int16_t>::max();
    }
    return static_cast<int16_t>(std::round(value));
}

size_t secondsToSamples(double seconds, uint32_t sample_rate) {
    if (!std::isfinite(seconds) || seconds < 0) {
        throw SoundProcessorError("time value must be non-negative");
    }
    const double samples = seconds * static_cast<double>(sample_rate);
    if (samples > static_cast<double>(std::numeric_limits<size_t>::max())) {
        throw SoundProcessorError("time value is too large");
    }
    return static_cast<size_t>(std::floor(samples));
}

size_t millisecondsToSamples(double milliseconds, uint32_t sample_rate) {
    if (!std::isfinite(milliseconds) || milliseconds < 0) {
        throw SoundProcessorError("time value must be non-negative");
    }
    return secondsToSamples(milliseconds / 1000.0, sample_rate);
}
