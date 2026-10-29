#pragma once

#include "sound_processor/args_parser.hpp"
#include "sound_processor/filters.hpp"
#include "sound_processor/pipeline.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

using FilterProducer = std::unique_ptr<IFilter> (*)(const FilterDescriptor&);

class CmdLineArgsToPipelineConverter {
public:
    void addFilterProducer(const std::string& filter_name, FilterProducer producer);
    Pipeline createPipeline(const std::vector<FilterDescriptor>& descriptors) const;

private:
    FilterProducer getFilterProducer(const std::string& filter_name) const;

    std::map<std::string, FilterProducer> producers_;
};
