#include <sys/stat.h>

#include <iostream>

#include "local/LinearProgram.h"
#include "local/ScriptGenerator.h"
#include "local/TclPRN.h"

static bool createDirectory(const std::string &path) {
    const int dir_err = ::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (-1 == dir_err && errno != EEXIST) {
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    // Vérification des paramètres
    if (argc != 6) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << argv[0] << " NUMBER_STAGE AREA_MAX FIRLS_DATA FIR1_DATA OUTPUT_FORMAT" << std::endl;
        std::exit(1);
    }

    // Définition des paramètres
    const std::int64_t nbStage = std::stoul(argv[1]);
    const double areaMax = std::strtod(argv[2], nullptr);
    const std::string firlsFile = argv[3];
    const std::string fir1File = argv[4];
    const std::string outputFormat = argv[5];

    // Création du répertoire de destination
    if (!createDirectory(outputFormat)) {
        std::cerr << "createDirectory(): create '" << outputFormat << "' directory: failed" << std::endl;
        std::exit(1);
    }

    std::cout << "### Start LP solver... ###" << std::endl;
    LinearProgram lp(nbStage, areaMax, firlsFile, fir1File, outputFormat);

    lp.printDebugFile();
    lp.printResults();
    lp.printResults("sol.txt");

    TclPRN tcl;
    tcl.generate(lp, outputFormat);

    ScriptGenerator::generateDeployScript(lp, outputFormat);
    ScriptGenerator::generateSimulationScript(lp, outputFormat);

    return 0;
}
