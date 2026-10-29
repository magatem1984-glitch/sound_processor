#pragma once

#include <stdexcept>
#include <string>

class SoundProcessorError : public std::runtime_error {
public:
    explicit SoundProcessorError(const std::string& message);
};
