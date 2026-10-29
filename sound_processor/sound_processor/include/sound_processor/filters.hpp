#pragma once

#include "sound_processor/waveform.hpp"

#include <cstddef>
#include <string>

class IFilter {
public:
    virtual ~IFilter() = default;
    virtual void apply(Waveform& sound) const = 0;
};

class AmplFilter : public IFilter {
public:
    explicit AmplFilter(double factor);
    void apply(Waveform& sound) const override;

private:
    double factor_ = 1;
};

class NormalizeFilter : public IFilter {
public:
    explicit NormalizeFilter(double peak);
    void apply(Waveform& sound) const override;

private:
    double peak_ = 1;
};

class SilenceFilter : public IFilter {
public:
    SilenceFilter(const std::string& unit, double start, double end);
    void apply(Waveform& sound) const override;

private:
    std::string unit_;
    double start_ = 0;
    double end_ = 0;
};

class TimestretchFilter : public IFilter {
public:
    explicit TimestretchFilter(double factor);
    void apply(Waveform& sound) const override;

private:
    double factor_ = 1;
};

class LowpassFilter : public IFilter {
public:
    explicit LowpassFilter(size_t window_size);
    void apply(Waveform& sound) const override;

private:
    size_t window_size_ = 1;
};

class GeneratorSinFilter : public IFilter {
public:
    GeneratorSinFilter(double frequency_hz, double duration_ms);
    void apply(Waveform& sound) const override;

private:
    double frequency_hz_ = 0;
    double duration_ms_ = 0;
};

class GeneratorAmFilter : public IFilter {
public:
    GeneratorAmFilter(double amplitude, double carrier_hz, double modulation_hz, double depth,
                      double duration_ms);
    void apply(Waveform& sound) const override;

private:
    double amplitude_ = 0;
    double carrier_hz_ = 0;
    double modulation_hz_ = 0;
    double depth_ = 0;
    double duration_ms_ = 0;
};

class GeneratorFmFilter : public IFilter {
public:
    GeneratorFmFilter(double amplitude, double carrier_hz, double modulation_hz, double deviation_hz,
                      double duration_ms);
    void apply(Waveform& sound) const override;

private:
    double amplitude_ = 0;
    double carrier_hz_ = 0;
    double modulation_hz_ = 0;
    double deviation_hz_ = 0;
    double duration_ms_ = 0;
};
