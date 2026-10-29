#include "sound_processor/args_parser.hpp"

#include "sound_processor/error.hpp"

#include <iostream>
#include <string>

namespace {

bool isOption(const std::string& token) {
    return token == "-i" || token == "-o" || token == "-f" || token == "-h" || token == "--help";
}

std::filesystem::path getValueAfterOption(int& index, int argc, char* argv[], const std::string& option) {
    if (index + 1 >= argc) {
        throw SoundProcessorError("missing value after " + option);
    }
    ++index;
    return argv[index];
}

}

ArgsParseResult ArgsParser::parse(int argc, char* argv[]) const {
    ArgsParseResult result;
    if (argc == 1) {
        result.show_help = true;
        return result;
    }

    for (int index = 1; index < argc; ++index) {
        const std::string token = argv[index];
        if (token == "-h" || token == "--help") {
            result.show_help = true;
            return result;
        }
        if (token == "-i") {
            result.input_path = getValueAfterOption(index, argc, argv, token);
            result.has_input = true;
            continue;
        }
        if (token == "-o") {
            result.output_path = getValueAfterOption(index, argc, argv, token);
            result.has_output = true;
            continue;
        }
        if (token == "-f") {
            if (index + 1 >= argc || isOption(argv[index + 1])) {
                throw SoundProcessorError("missing filter name after -f");
            }
            FilterDescriptor descriptor;
            descriptor.name = argv[++index];
            while (index + 1 < argc && !isOption(argv[index + 1])) {
                descriptor.params.emplace_back(argv[++index]);
            }
            result.filters.push_back(std::move(descriptor));
            continue;
        }
        throw SoundProcessorError("unknown argument: " + token);
    }

    return result;
}

void printHelp() {
    std::cout << "Usage:\n"
              << "  sound_processor [-i input.wav] [-o output.wav] [-f filter [params] ...]\n\n"
              << "Examples:\n"
              << "  sound_processor -i input.wav -o output.wav -f ampl 0.8 -f timestretch 2\n"
              << "  sound_processor -o output.wav -f generator sin 440 2000 -f ampl 1.1\n\n"
              << "Filters:\n"
              << "  -f ampl factor\n"
              << "  -f normalize [peak]\n"
              << "  -f silence sec start end\n"
              << "  -f silence ms start end\n"
              << "  -f timestretch factor\n"
              << "  -f lowpass window_size\n"
              << "  -f generator sin frequency_hz duration_ms\n"
              << "  -f generator am amplitude carrier_hz modulation_hz depth duration_ms\n"
              << "  -f generator fm amplitude carrier_hz modulation_hz deviation_hz duration_ms\n";
}
