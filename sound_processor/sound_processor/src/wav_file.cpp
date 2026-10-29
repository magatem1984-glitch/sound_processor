#include "sound_processor/wav_file.hpp"

#include "sound_processor/error.hpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace {

uint16_t readUInt16(std::istream& input) {
    std::array<unsigned char, 2> bytes{};
    input.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
    if (!input) {
        throw SoundProcessorError("unexpected end of wav file");
    }
    return static_cast<uint16_t>(bytes[0]) | (static_cast<uint16_t>(bytes[1]) << 8);
}

uint32_t readUInt32(std::istream& input) {
    std::array<unsigned char, 4> bytes{};
    input.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
    if (!input) {
        throw SoundProcessorError("unexpected end of wav file");
    }
    return static_cast<uint32_t>(bytes[0]) | (static_cast<uint32_t>(bytes[1]) << 8) |
           (static_cast<uint32_t>(bytes[2]) << 16) | (static_cast<uint32_t>(bytes[3]) << 24);
}

void writeUInt16(std::ostream& output, uint16_t value) {
    const std::array<char, 2> bytes{static_cast<char>(value & 0xFF),
                                   static_cast<char>((value >> 8) & 0xFF)};
    output.write(bytes.data(), bytes.size());
}

void writeUInt32(std::ostream& output, uint32_t value) {
    const std::array<char, 4> bytes{static_cast<char>(value & 0xFF),
                                   static_cast<char>((value >> 8) & 0xFF),
                                   static_cast<char>((value >> 16) & 0xFF),
                                   static_cast<char>((value >> 24) & 0xFF)};
    output.write(bytes.data(), bytes.size());
}

std::string readChunkId(std::istream& input) {
    std::array<char, 4> id{};
    input.read(id.data(), id.size());
    if (!input) {
        throw SoundProcessorError("unexpected end of wav file");
    }
    return std::string(id.data(), id.size());
}

void writeChunkId(std::ostream& output, const std::string& id) {
    output.write(id.data(), 4);
}

void skipBytes(std::istream& input, uint32_t size) {
    input.seekg(size, std::ios::cur);
    if (!input) {
        throw SoundProcessorError("invalid wav chunk size");
    }
}

void skipPadding(std::istream& input, uint32_t chunk_size) {
    if (chunk_size % 2 == 1) {
        skipBytes(input, 1);
    }
}

void validateFormat(const WavFormat& format) {
    if (format.audio_format != 1) {
        throw SoundProcessorError("only PCM wav files are supported");
    }
    if (format.channels != 1) {
        throw SoundProcessorError("only mono wav files are supported");
    }
    if (format.sample_rate != 44100) {
        throw SoundProcessorError("only 44100 Hz wav files are supported");
    }
    if (format.bits_per_sample != 16) {
        throw SoundProcessorError("only 16-bit wav files are supported");
    }
    const uint16_t expected_block_align =
        static_cast<uint16_t>(format.channels * format.bits_per_sample / 8);
    const uint32_t expected_byte_rate = format.sample_rate * expected_block_align;
    if (format.block_align != expected_block_align || format.byte_rate != expected_byte_rate) {
        throw SoundProcessorError("wav file format fields are inconsistent");
    }
}

}

Waveform WavFile::read(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw SoundProcessorError("cannot open input file: " + path.string());
    }

    if (readChunkId(input) != "RIFF") {
        throw SoundProcessorError("input file is not a RIFF file: " + path.string());
    }
    readUInt32(input);
    if (readChunkId(input) != "WAVE") {
        throw SoundProcessorError("input file is not a WAVE file: " + path.string());
    }

    WavFormat format;
    std::vector<int16_t> samples;
    bool has_format = false;
    bool has_data = false;

    while (input.peek() != std::char_traits<char>::eof()) {
        const std::string chunk_id = readChunkId(input);
        const uint32_t chunk_size = readUInt32(input);

        if (chunk_id == "fmt ") {
            if (chunk_size < 16) {
                throw SoundProcessorError("wav fmt chunk is too small: " + path.string());
            }
            format.audio_format = readUInt16(input);
            format.channels = readUInt16(input);
            format.sample_rate = readUInt32(input);
            format.byte_rate = readUInt32(input);
            format.block_align = readUInt16(input);
            format.bits_per_sample = readUInt16(input);
            if (chunk_size > 16) {
                skipBytes(input, chunk_size - 16);
            }
            skipPadding(input, chunk_size);
            has_format = true;
        } else if (chunk_id == "data") {
            if (!has_format) {
                throw SoundProcessorError("wav data chunk appears before fmt chunk: " + path.string());
            }
            validateFormat(format);
            if (chunk_size % sizeof(int16_t) != 0) {
                throw SoundProcessorError("wav data chunk has invalid size: " + path.string());
            }
            samples.resize(chunk_size / sizeof(int16_t));
            input.read(reinterpret_cast<char*>(samples.data()), chunk_size);
            if (!input) {
                throw SoundProcessorError("cannot read wav samples: " + path.string());
            }
            skipPadding(input, chunk_size);
            has_data = true;
        } else {
            skipBytes(input, chunk_size);
            skipPadding(input, chunk_size);
        }
    }

    if (!has_format) {
        throw SoundProcessorError("wav fmt chunk is not found: " + path.string());
    }
    validateFormat(format);
    if (!has_data) {
        samples.clear();
    }
    return Waveform(format, std::move(samples));
}

void WavFile::write(const std::filesystem::path& path, const Waveform& waveform) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw SoundProcessorError("cannot open output file: " + path.string());
    }

    const WavFormat& format = waveform.getFormat();
    validateFormat(format);
    const size_t bytes_count = waveform.getSamples().size() * sizeof(int16_t);
    if (bytes_count > std::numeric_limits<uint32_t>::max() - 36) {
        throw SoundProcessorError("wav file is too large");
    }
    const uint32_t data_size = static_cast<uint32_t>(bytes_count);
    const uint32_t riff_size = 36 + data_size;

    writeChunkId(output, "RIFF");
    writeUInt32(output, riff_size);
    writeChunkId(output, "WAVE");
    writeChunkId(output, "fmt ");
    writeUInt32(output, 16);
    writeUInt16(output, format.audio_format);
    writeUInt16(output, format.channels);
    writeUInt32(output, format.sample_rate);
    writeUInt32(output, format.byte_rate);
    writeUInt16(output, format.block_align);
    writeUInt16(output, format.bits_per_sample);
    writeChunkId(output, "data");
    writeUInt32(output, data_size);
    output.write(reinterpret_cast<const char*>(waveform.getSamples().data()), data_size);
    if (!output) {
        throw SoundProcessorError("cannot write output file: " + path.string());
    }
}
