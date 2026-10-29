#include "sound_processor/cmd_line_args_to_pipeline_converter.hpp"

#include "sound_processor/error.hpp"

#include <utility>

void CmdLineArgsToPipelineConverter::addFilterProducer(const std::string& filter_name,
                                                       FilterProducer producer) {
    producers_[filter_name] = producer;
}

Pipeline CmdLineArgsToPipelineConverter::createPipeline(
    const std::vector<FilterDescriptor>& descriptors) const {
    Pipeline pipeline;
    for (const FilterDescriptor& descriptor : descriptors) {
        FilterProducer producer = getFilterProducer(descriptor.name);
        if (producer == nullptr) {
            throw SoundProcessorError("unknown filter: " + descriptor.name);
        }
        pipeline.addFilter(producer(descriptor));
    }
    return pipeline;
}

FilterProducer CmdLineArgsToPipelineConverter::getFilterProducer(const std::string& filter_name) const {
    const auto iterator = producers_.find(filter_name);
    if (iterator == producers_.end()) {
        return nullptr;
    }
    return iterator->second;
}
