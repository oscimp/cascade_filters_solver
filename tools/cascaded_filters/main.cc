#include <cstdlib>
#include <iostream>
#include <utility>

#include <dsps/FileSink.h>
#include <dsps/FileSource.h>
#include <dsps/Fir.h>
#include <dsps/Shifter.h>
#include <dsps/Utils.h>

struct FilterStage {
    std::string filterName;
    std::int64_t shift;
    std::int64_t maxNobFir;
};

int main(int argc, char *argv[]) {
    if (argc < 5 || (argc - 3) % 3 != 0) {
        std::cerr << "Wrong parameters" << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << argv[0] << " RAW_DATA_FILE OUTPUT_FILE FIR1_FILE SHIFT1_VALUE NOB_MAX_FIR [FIRN_FILE SHIFTN_VALUE NOB_MAX_FIR ...]" << std::endl;

        return 1;
    }

    constexpr std::int64_t N = 2048;

    // Get the input file
    std::string rawDataFile = argv[1];
    std::string outputFile = argv[2];

    // Get the list of filters
    std::vector<FilterStage> filters;
    for (int i = 0; i < argc - 3; i += 3) {
        FilterStage stage = {argv[3 + i], std::atoll(argv[4 + i]), std::atoll(argv[5 + i])};
        filters.push_back(stage);
    }

    // Create the DSP
    FileSource source(rawDataFile, FileSource::FileFormat::BinaryInteger);

    // Create filter stages
    Task *previousSource = &source;
    std::vector<Task*> garbages;
    for (std::size_t i = 0; i < filters.size(); ++i) {
        FilterStage &stage = filters[i];
        Fir<std::int64_t> *fir = new Fir<std::int64_t>(stage.filterName, 1, stage.maxNobFir);
        garbages.push_back(fir);
        Task::connect(*previousSource, *fir);

        if (stage.shift > 0) {
            Shifter<std::int64_t> *shifter = new Shifter<std::int64_t>(stage.shift);
            garbages.push_back(shifter);
            Task::connect(*fir, *shifter);

            previousSource = shifter;
        }
        else {
            previousSource = fir;
        }
    }

    // Create output
    FileSink<std::int64_t> sink(outputFile);
    Task::connect(*previousSource, sink);

    for (std::size_t i = 0; i < 2000; ++i) {
        DSP::processing({ &source }, { &sink }, N);
    }

    // std::string inputFile = argv[1];
    // std::string coeffFile1 = argv[2];
    // std::string coeffFile2 = argv[3];
    // constexpr std::int64_t N = 2048;
    // constexpr std::int64_t n = N * 1000;
    //
    // for (int i = 14; i >= 0; --i) {
    //     FileSource source(inputFile, FileSource::FileFormat::BinaryInteger);
    //     Fir<std::int64_t> fir1(coeffFile1, 1);
    //     Task::connect(source, fir1);
    //
    //     Splitter<std::int64_t> splitter1(2);
    //     FileSink<std::int64_t> sink1("simulation_post_fir1_" + std::to_string(i) + ".bin", true);
    //     Shifter<std::int64_t> shifter(i);
    //     Task::connect(fir1, splitter1);
    //     Task::connect(splitter1, shifter);
    //     Task::connect(splitter1, 1, sink1, 0);
    //
    //     Splitter<std::int64_t> splitter2(2);
    //     FileSink<std::int64_t> sink2("simulation_post_shift" + std::to_string(i) + ".bin", true);
    //     Fir<std::int64_t> fir2(coeffFile2, 1);
    //     Task::connect(shifter, splitter2);
    //     Task::connect(splitter2, fir2);
    //     Task::connect(splitter2, 1, sink2, 0);
    //
    //     FileSink<std::int64_t> sink3("simulation_post_fir2_" + std::to_string(i) + ".bin", true);
    //
    //     Task::connect(fir2, sink3);
    //
    //     DSP::processing({ &source }, { &sink1, &sink2, &sink3 }, n);
    // }

    return 0;
}
