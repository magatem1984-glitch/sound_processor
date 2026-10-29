#pragma once

#include "sound_processor/args_parser.hpp"
#include "sound_processor/filters.hpp"

#include <memory>

namespace FilterProducers {

std::unique_ptr<IFilter> createAmplFilter(const FilterDescriptor& descriptor);
std::unique_ptr<IFilter> createNormalizeFilter(const FilterDescriptor& descriptor);
std::unique_ptr<IFilter> createSilenceFilter(const FilterDescriptor& descriptor);
std::unique_ptr<IFilter> createTimestretchFilter(const FilterDescriptor& descriptor);
std::unique_ptr<IFilter> createLowpassFilter(const FilterDescriptor& descriptor);
std::unique_ptr<IFilter> createGeneratorFilter(const FilterDescriptor& descriptor);

}
