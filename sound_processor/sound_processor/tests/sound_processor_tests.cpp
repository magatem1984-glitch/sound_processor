#include "sound_processor/args_parser.hpp"
#include "sound_processor/cmd_line_args_to_pipeline_converter.hpp"
#include "sound_processor/filters.hpp"
#include "sound_processor/std_filter_producers.hpp"
#include "sound_processor/wav_file.hpp"
#include "sound_processor/waveform.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void check(bool value, const std::string& message) {
    if (!value) {
        throw std::runtime_error(message);
    }
}

int maxAbsSample(const std::vector<int16_t>& samples) {
    int result = 0;
    for (int16_t sample : samples) {
        const int value = sample;
        const int absolute = value == -32768 ? 32768 : std::abs(value);
        result = std::max(result, absolute);
    }
    return result;
}

void testAmpl() {
    Waveform sound({1000, -2000, 3000});
    const AmplFilter filter(2.0);
    filter.apply(sound);
    check((sound.getSamples() == std::vector<int16_t>{2000, -4000, 6000}), "ampl filter failed");
}

void testNormalize() {
    Waveform sound({1000, -2000, 500});
    const NormalizeFilter filter(0.5);
    filter.apply(sound);
    check(std::abs(maxAbsSample(sound.getSamples()) - 16384) <= 1, "normalize filter failed");
}

void testSilence() {
    Waveform sound({1, 2, 3});
    const SilenceFilter filter("ms", 1.0, 2.0);
    filter.apply(sound);
    check(sound.getSampleCount() == 47, "silence size failed");
    check(sound.getSamples()[0] == 1 && sound.getSamples()[1] == 2 && sound.getSamples()[2] == 3,
          "silence insertion failed");
}

void testTimestretch() {
    Waveform sound({0, 10, 20});
    const TimestretchFilter filter(2.0);
    filter.apply(sound);
    check((sound.getSamples() == std::vector<int16_t>{0, 5, 10, 15, 20, 20}),
          "timestretch filter failed");
}

void testLowpass() {
    Waveform sound({0, 30, 0});
    const LowpassFilter filter(3);
    filter.apply(sound);
    check((sound.getSamples() == std::vector<int16_t>{10, 10, 10}), "lowpass filter failed");
}

void testGeneratorSin() {
    Waveform sound({1, 2, 3});
    const GeneratorSinFilter filter(440.0, 1000.0);
    filter.apply(sound);
    check(sound.getSampleCount() == 44100, "generator sin size failed");
    check(sound.getSamples()[0] == 0, "generator sin first sample failed");
}

void testParser() {
    ArgsParser parser;
    const char* raw_args[] = {"sound_processor", "-i", "input.wav", "-o", "output.wav",
                              "-f", "ampl", "0.8", "-f", "timestretch", "2"};
    std::vector<char*> args;
    for (const char* value : raw_args) {
        args.push_back(const_cast<char*>(value));
    }
    const ArgsParseResult result = parser.parse(static_cast<int>(args.size()), args.data());
    check(result.has_input && result.has_output, "parser files failed");
    check(result.input_path == "input.wav" && result.output_path == "output.wav", "parser paths failed");
    check(result.filters.size() == 2, "parser filters count failed");
    check(result.filters[0].name == "ampl" && result.filters[0].params[0] == "0.8",
          "parser ampl failed");
}

void testConverter() {
    CmdLineArgsToPipelineConverter converter;
    converter.addFilterProducer("ampl", FilterProducers::createAmplFilter);
    converter.addFilterProducer("timestretch", FilterProducers::createTimestretchFilter);
    const std::vector<FilterDescriptor> descriptors{{"ampl", {"0.5"}}, {"timestretch", {"2"}}};
    Pipeline pipeline = converter.createPipeline(descriptors);
    Waveform sound({100, 200});
    pipeline.apply(sound);
    check((sound.getSamples() == std::vector<int16_t>{50, 75, 100, 100}), "converter failed");
}

void testWavFile() {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "sound_processor_test.wav";
    const Waveform written({1, -2, 3000});
    WavFile::write(path, written);
    const Waveform read = WavFile::read(path);
    std::filesystem::remove(path);
    check(read.getFormat().audio_format == 1, "wav format failed");
    check(read.getFormat().channels == 1, "wav channels failed");
    check(read.getFormat().sample_rate == 44100, "wav sample rate failed");
    check((read.getSamples() == std::vector<int16_t>{1, -2, 3000}), "wav samples failed");
}

}

int main() {
    testAmpl();
    testNormalize();
    testSilence();
    testTimestretch();
    testLowpass();
    testGeneratorSin();
    testParser();
    testConverter();
    testWavFile();
    return 0;
}
