#include "sound_processor/filters.hpp"

#include "sound_processor/error.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kMaxSampleValue = 32767.0;

void checkFinite(double value, const std::string& name) {
    if (!std::isfinite(value)) {
        throw SoundProcessorError(name + " must be finite");
    }
}

void checkNonNegative(double value, const std::string& name) {
    checkFinite(value, name);
    if (value < 0) {
        throw SoundProcessorError(name + " must be non-negative");
    }
}

void checkUnitInterval(double value, const std::string& name) {
    checkFinite(value, name);
    if (value < 0 || value > 1) {
        throw SoundProcessorError(name + " must be in range [0, 1]");
    }
}

size_t durationMsToSamples(double duration_ms, uint32_t sample_rate) {
    checkNonNegative(duration_ms, "duration_ms");
    const double samples = duration_ms * static_cast<double>(sample_rate) / 1000.0;
    if (samples > static_cast<double>(std::numeric_limits<size_t>::max())) {
        throw SoundProcessorError("duration_ms is too large");
    }
    return static_cast<size_t>(std::round(samples));
}

std::vector<int16_t> generateSamples(double duration_ms, double amplitude,
                                     const std::function<double(double)>& value_at_time) {
    const WavFormat format;
    const size_t size = durationMsToSamples(duration_ms, format.sample_rate);
    std::vector<int16_t> result(size);
    for (size_t index = 0; index < size; ++index) {
        const double time = static_cast<double>(index) / static_cast<double>(format.sample_rate);
        result[index] = clampToSample(amplitude * kMaxSampleValue * value_at_time(time));
    }
    return result;
}

int sampleAbs(int16_t sample) {
    const int value = sample;
    if (value == std::numeric_limits<int16_t>::min()) {
        return 32768;
    }
    return std::abs(value);
}

}

AmplFilter::AmplFilter(double factor) : factor_(factor) {
    checkNonNegative(factor_, "factor");
}

void AmplFilter::apply(Waveform& sound) const {
    for (int16_t& sample : sound.getSamples()) {
        sample = clampToSample(static_cast<double>(sample) * factor_);
    }
}

NormalizeFilter::NormalizeFilter(double peak) : peak_(peak) {
    checkUnitInterval(peak_, "peak");
}

void NormalizeFilter::apply(Waveform& sound) const {
    int current_peak = 0;
    for (int16_t sample : sound.getSamples()) {
        current_peak = std::max(current_peak, sampleAbs(sample));
    }
    if (sound.empty() || current_peak == 0) {
        return;
    }
    const double scale = peak_ * kMaxSampleValue / static_cast<double>(current_peak);
    for (int16_t& sample : sound.getSamples()) {
        sample = clampToSample(static_cast<double>(sample) * scale);
    }
}

SilenceFilter::SilenceFilter(const std::string& unit, double start, double end)
    : unit_(unit), start_(start), end_(end) {
    if (unit_ != "sec" && unit_ != "ms") {
        throw SoundProcessorError("silence unit must be sec or ms");
    }
    checkNonNegative(start_, "start");
    checkNonNegative(end_, "end");
    if (end_ < start_) {
        throw SoundProcessorError("end must be greater than or equal to start");
    }
}

void SilenceFilter::apply(Waveform& sound) const {
    const uint32_t sample_rate = sound.getFormat().sample_rate;
    size_t start_sample = 0;
    size_t end_sample = 0;
    if (unit_ == "sec") {
        start_sample = secondsToSamples(start_, sample_rate);
        end_sample = secondsToSamples(end_, sample_rate);
    } else {
        start_sample = millisecondsToSamples(start_, sample_rate);
        end_sample = millisecondsToSamples(end_, sample_rate);
    }
    const size_t duration = end_sample - start_sample;
    const size_t insertion_position = std::min(start_sample, sound.getSampleCount());
    sound.getSamples().insert(sound.getSamples().begin() + static_cast<std::ptrdiff_t>(insertion_position),
                              duration, 0);
}

TimestretchFilter::TimestretchFilter(double factor) : factor_(factor) {
    checkFinite(factor_, "factor");
    if (factor_ <= 0) {
        throw SoundProcessorError("factor must be positive");
    }
}

void TimestretchFilter::apply(Waveform& sound) const {
    const std::vector<int16_t> original = sound.getSamples();
    if (original.empty()) {
        return;
    }
    const double new_size_value = std::round(static_cast<double>(original.size()) * factor_);
    if (new_size_value > static_cast<double>(std::numeric_limits<size_t>::max())) {
        throw SoundProcessorError("resulting signal is too large");
    }
    const size_t new_size = static_cast<size_t>(new_size_value);
    std::vector<int16_t> result(new_size);
    for (size_t index = 0; index < new_size; ++index) {
        const double position = static_cast<double>(index) / factor_;
        size_t left = static_cast<size_t>(std::floor(position));
        if (left >= original.size()) {
            left = original.size() - 1;
        }
        if (left + 1 >= original.size()) {
            result[index] = original[left];
            continue;
        }
        const double fraction = position - static_cast<double>(left);
        const double value = static_cast<double>(original[left]) * (1.0 - fraction) +
                             static_cast<double>(original[left + 1]) * fraction;
        result[index] = clampToSample(value);
    }
    sound.setSamples(std::move(result));
}

LowpassFilter::LowpassFilter(size_t window_size) : window_size_(window_size) {
    if (window_size_ < 1 || window_size_ % 2 == 0) {
        throw SoundProcessorError("window_size must be odd and not less than 1");
    }
}

void LowpassFilter::apply(Waveform& sound) const {
    const std::vector<int16_t> original = sound.getSamples();
    if (original.empty()) {
        return;
    }
    std::vector<int16_t> result(original.size());
    const ptrdiff_t radius = static_cast<ptrdiff_t>(window_size_ / 2);
    const ptrdiff_t last_index = static_cast<ptrdiff_t>(original.size() - 1);
    for (size_t index = 0; index < original.size(); ++index) {
        double sum = 0;
        for (ptrdiff_t shift = -radius; shift <= radius; ++shift) {
            const ptrdiff_t current = static_cast<ptrdiff_t>(index) + shift;
            const ptrdiff_t clamped = std::clamp(current, ptrdiff_t{0}, last_index);
            sum += static_cast<double>(original[static_cast<size_t>(clamped)]);
        }
        result[index] = clampToSample(sum / static_cast<double>(window_size_));
    }
    sound.setSamples(std::move(result));
}

GeneratorSinFilter::GeneratorSinFilter(double frequency_hz, double duration_ms)
    : frequency_hz_(frequency_hz), duration_ms_(duration_ms) {
    checkNonNegative(frequency_hz_, "frequency_hz");
    checkNonNegative(duration_ms_, "duration_ms");
}

void GeneratorSinFilter::apply(Waveform& sound) const {
    sound.setFormat(WavFormat{});
    sound.setSamples(generateSamples(duration_ms_, 1.0, [this](double time) {
        return std::sin(2.0 * kPi * frequency_hz_ * time);
    }));
}

GeneratorAmFilter::GeneratorAmFilter(double amplitude, double carrier_hz, double modulation_hz,
                                     double depth, double duration_ms)
    : amplitude_(amplitude),
      carrier_hz_(carrier_hz),
      modulation_hz_(modulation_hz),
      depth_(depth),
      duration_ms_(duration_ms) {
    checkUnitInterval(amplitude_, "amplitude");
    checkNonNegative(carrier_hz_, "carrier_hz");
    checkNonNegative(modulation_hz_, "modulation_hz");
    checkUnitInterval(depth_, "depth");
    checkNonNegative(duration_ms_, "duration_ms");
}

void GeneratorAmFilter::apply(Waveform& sound) const {
    sound.setFormat(WavFormat{});
    sound.setSamples(generateSamples(duration_ms_, amplitude_, [this](double time) {
        const double envelope = 1.0 + depth_ * std::sin(2.0 * kPi * modulation_hz_ * time);
        const double carrier = std::sin(2.0 * kPi * carrier_hz_ * time);
        return envelope * carrier;
    }));
}

GeneratorFmFilter::GeneratorFmFilter(double amplitude, double carrier_hz, double modulation_hz,
                                     double deviation_hz, double duration_ms)
    : amplitude_(amplitude),
      carrier_hz_(carrier_hz),
      modulation_hz_(modulation_hz),
      deviation_hz_(deviation_hz),
      duration_ms_(duration_ms) {
    checkUnitInterval(amplitude_, "amplitude");
    checkNonNegative(carrier_hz_, "carrier_hz");
    checkFinite(modulation_hz_, "modulation_hz");
    if (modulation_hz_ <= 0) {
        throw SoundProcessorError("modulation_hz must be positive");
    }
    checkNonNegative(deviation_hz_, "deviation_hz");
    checkNonNegative(duration_ms_, "duration_ms");
}

void GeneratorFmFilter::apply(Waveform& sound) const {
    sound.setFormat(WavFormat{});
    sound.setSamples(generateSamples(duration_ms_, amplitude_, [this](double time) {
        const double phase = 2.0 * kPi * carrier_hz_ * time +
                             (deviation_hz_ / modulation_hz_) *
                                 std::sin(2.0 * kPi * modulation_hz_ * time);
        return std::sin(phase);
    }));
}
