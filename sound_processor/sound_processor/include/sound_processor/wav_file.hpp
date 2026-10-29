#pragma once

#include "sound_processor/waveform.hpp"

#include <filesystem>

class WavFile {
public:
    static Waveform read(const std::filesystem::path& path);
    static void write(const std::filesystem::path& path, const Waveform& waveform);
};
