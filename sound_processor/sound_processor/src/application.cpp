#include "sound_processor/application.hpp"

#include "sound_processor/args_parser.hpp"
#include "sound_processor/std_filter_producers.hpp"
#include "sound_processor/wav_file.hpp"
#include "sound_processor/waveform.hpp"

void Application::configure() {
    cmd_line_args_to_pipeline_converter_.addFilterProducer("ampl", FilterProducers::createAmplFilter);
    cmd_line_args_to_pipeline_converter_.addFilterProducer("normalize", FilterProducers::createNormalizeFilter);
    cmd_line_args_to_pipeline_converter_.addFilterProducer("silence", FilterProducers::createSilenceFilter);
    cmd_line_args_to_pipeline_converter_.addFilterProducer("timestretch",
                                                           FilterProducers::createTimestretchFilter);
    cmd_line_args_to_pipeline_converter_.addFilterProducer("lowpass", FilterProducers::createLowpassFilter);
    cmd_line_args_to_pipeline_converter_.addFilterProducer("generator", FilterProducers::createGeneratorFilter);
}

int Application::start(int argc, char* argv[]) const {
    const ArgsParser parser;
    const ArgsParseResult args = parser.parse(argc, argv);
    if (args.show_help) {
        printHelp();
        return 0;
    }

    Waveform sound;
    if (args.has_input) {
        sound = WavFile::read(args.input_path);
    }

    Pipeline pipeline = cmd_line_args_to_pipeline_converter_.createPipeline(args.filters);
    pipeline.apply(sound);

    if (args.has_output) {
        WavFile::write(args.output_path, sound);
    }
    return 0;
}
