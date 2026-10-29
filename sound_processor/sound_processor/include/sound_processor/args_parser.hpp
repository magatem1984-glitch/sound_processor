#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct FilterDescriptor {
    std::string name;
    std::vector<std::string> params;
};

struct ArgsParseResult {
    bool show_help = false;
    bool has_input = false;
    bool has_output = false;
    std::filesystem::path input_path;
    std::filesystem::path output_path;
    std::vector<FilterDescriptor> filters;
};

class ArgsParser {
public:
    ArgsParseResult parse(int argc, char* argv[]) const;
};

void printHelp();
