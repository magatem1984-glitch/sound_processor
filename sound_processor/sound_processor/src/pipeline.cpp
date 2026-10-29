#include "sound_processor/pipeline.hpp"

#include <utility>

void Pipeline::addFilter(std::unique_ptr<IFilter> filter) {
    filters_.push_back(std::move(filter));
}

void Pipeline::apply(Waveform& sound) const {
    for (const auto& filter : filters_) {
        filter->apply(sound);
    }
}

size_t Pipeline::size() const {
    return filters_.size();
}
