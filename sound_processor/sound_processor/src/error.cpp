#include "sound_processor/error.hpp"

SoundProcessorError::SoundProcessorError(const std::string& message) : std::runtime_error(message) {
}
