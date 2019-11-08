/* Cascade filters solver - Solver to choose the best configuration to design a
 * cascade of filters
 * Copyright (C) 2019  Arthur HUGEAT
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */

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
    try {
      LinearProgram lp(nbStage, areaMax, firlsFile, fir1File, outputFormat);

      lp.printDebugFile();
      lp.printResults();
      lp.printResults("sol.txt");

      TclPRN tcl;
      tcl.generate(lp, outputFormat);

      ScriptGenerator::generateDeployScript(lp, outputFormat, "prn");
      ScriptGenerator::generateSimulationScript(lp, outputFormat);
    } catch (GRBException e) {
      std::cout << e.getMessage() << std::endl;
    }


    return 0;
}
