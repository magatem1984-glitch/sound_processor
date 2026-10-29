#include "sound_processor/std_filter_producers.hpp"

#include "sound_processor/error.hpp"

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <string>

namespace {

void checkParamCount(const FilterDescriptor& descriptor, size_t expected) {
    if (descriptor.params.size() != expected) {
        throw SoundProcessorError("filter " + descriptor.name + " has invalid parameters count");
    }
}

void checkParamCountRange(const FilterDescriptor& descriptor, size_t minimum, size_t maximum) {
    if (descriptor.params.size() < minimum || descriptor.params.size() > maximum) {
        throw SoundProcessorError("filter " + descriptor.name + " has invalid parameters count");
    }
}

double parseDouble(const std::string& text, const std::string& name) {
    char* end = nullptr;
    errno = 0;
    const double value = std::strtod(text.c_str(), &end);
    if (errno != 0 || end == text.c_str() || *end != '\0') {
        throw SoundProcessorError("invalid floating point value for " + name + ": " + text);
    }
    return value;
}

size_t parseSize(const std::string& text, const std::string& name) {
    char* end = nullptr;
    errno = 0;
    const unsigned long long value = std::strtoull(text.c_str(), &end, 10);
    if (text.empty() || errno != 0 || end == text.c_str() || *end != '\0' || text.front() == '-') {
        throw SoundProcessorError("invalid integer value for " + name + ": " + text);
    }
    if (value > static_cast<unsigned long long>(std::numeric_limits<size_t>::max())) {
        throw SoundProcessorError("integer value for " + name + " is too large");
    }
    return static_cast<size_t>(value);
}

}

namespace FilterProducers {

std::unique_ptr<IFilter> createAmplFilter(const FilterDescriptor& descriptor) {
    checkParamCount(descriptor, 1);
    return std::make_unique<AmplFilter>(parseDouble(descriptor.params[0], "factor"));
}

std::unique_ptr<IFilter> createNormalizeFilter(const FilterDescriptor& descriptor) {
    checkParamCountRange(descriptor, 0, 1);
    double peak = 1.0;
    if (!descriptor.params.empty()) {
        peak = parseDouble(descriptor.params[0], "peak");
    }
    return std::make_unique<NormalizeFilter>(peak);
}

std::unique_ptr<IFilter> createSilenceFilter(const FilterDescriptor& descriptor) {
    checkParamCount(descriptor, 3);
    return std::make_unique<SilenceFilter>(descriptor.params[0], parseDouble(descriptor.params[1], "start"),
                                           parseDouble(descriptor.params[2], "end"));
}

std::unique_ptr<IFilter> createTimestretchFilter(const FilterDescriptor& descriptor) {
    checkParamCount(descriptor, 1);
    return std::make_unique<TimestretchFilter>(parseDouble(descriptor.params[0], "factor"));
}

std::unique_ptr<IFilter> createLowpassFilter(const FilterDescriptor& descriptor) {
    checkParamCount(descriptor, 1);
    return std::make_unique<LowpassFilter>(parseSize(descriptor.params[0], "window_size"));
}

std::unique_ptr<IFilter> createGeneratorFilter(const FilterDescriptor& descriptor) {
    if (descriptor.params.empty()) {
        throw SoundProcessorError("generator type is not specified");
    }
    const std::string& type = descriptor.params[0];
    if (type == "sin") {
        checkParamCount(descriptor, 3);
        return std::make_unique<GeneratorSinFilter>(parseDouble(descriptor.params[1], "frequency_hz"),
                                                    parseDouble(descriptor.params[2], "duration_ms"));
    }
    if (type == "am") {
        checkParamCount(descriptor, 6);
        return std::make_unique<GeneratorAmFilter>(
            parseDouble(descriptor.params[1], "amplitude"), parseDouble(descriptor.params[2], "carrier_hz"),
            parseDouble(descriptor.params[3], "modulation_hz"), parseDouble(descriptor.params[4], "depth"),
            parseDouble(descriptor.params[5], "duration_ms"));
    }
    if (type == "fm") {
        checkParamCount(descriptor, 6);
        return std::make_unique<GeneratorFmFilter>(
            parseDouble(descriptor.params[1], "amplitude"), parseDouble(descriptor.params[2], "carrier_hz"),
            parseDouble(descriptor.params[3], "modulation_hz"),
            parseDouble(descriptor.params[4], "deviation_hz"), parseDouble(descriptor.params[5], "duration_ms"));
    }
    throw SoundProcessorError("unknown generator type: " + type);
}

}
