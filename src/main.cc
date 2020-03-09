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

#include "local/MaximizeRejection.h"
#include "local/MinimizeArea.h"
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
    if (argc != 7) {
        std::cerr << "Missing parameter" << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << argv[0] << " --max_rej|--min_area NUMBER_STAGE CONSTRAINT_LIMIT FIRLS_DATA FIR1_DATA EXPERIMENT_NAME" << std::endl;
        std::exit(1);
    }

    // Définition des paramètres
    std::string milpOption = std::string(argv[1]);
    const std::int64_t nbStage = std::stoul(argv[2]);
    const double constraintLimit = std::strtod(argv[3], nullptr);
    const std::string firlsFile = argv[4];
    const std::string fir1File = argv[5];
    const std::string experimentName = argv[6];

    // Select the right problem
    QuadraticProgram *milp = nullptr;
    if (milpOption == "--max_rej") {
        milp = new MaximizeRejection(nbStage, constraintLimit, firlsFile, fir1File, experimentName);
    }
    else if (milpOption == "--min_area") {
        milp = new MinimizeArea(nbStage, constraintLimit, firlsFile, fir1File, experimentName);
    }
    else {
        std::cerr << "'" << milpOption << "' is not a valid option" << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << argv[0] << " --max_rej|--min_area NUMBER_STAGE AREA_MAX FIRLS_DATA FIR1_DATA EXPERIMENT_NAME" << std::endl;
        std::exit(1);
    }

    // Création du répertoire de destination
    if (!createDirectory(experimentName)) {
        std::cerr << "createDirectory(): create '" << experimentName << "' directory: failed" << std::endl;
        std::exit(1);
    }

    std::cout << "### Start LP solver... ###" << std::endl;
    try {
      milp->printDebugFiles();
      milp->printResults();
      milp->printResults("sol.txt");

      TclPRN tcl;
      tcl.generate(*milp, experimentName);

      ScriptGenerator::generateDeployScript(*milp, experimentName, "prn");
      ScriptGenerator::generateSimulationScript(*milp, experimentName);
    } catch (GRBException e) {
      std::cerr << e.getMessage() << std::endl;
    }

    delete milp;

    return 0;
}
