#pragma once

#include "sound_processor/filters.hpp"
#include "sound_processor/waveform.hpp"

#include <memory>
#include <vector>

class Pipeline {
public:
    void addFilter(std::unique_ptr<IFilter> filter);
    void apply(Waveform& sound) const;
    size_t size() const;

private:
    std::vector<std::unique_ptr<IFilter>> filters_;
};
