#pragma once

#include "sound_processor/cmd_line_args_to_pipeline_converter.hpp"

class Application {
public:
    void configure();
    int start(int argc, char* argv[]) const;

private:
    CmdLineArgsToPipelineConverter cmd_line_args_to_pipeline_converter_;
};
