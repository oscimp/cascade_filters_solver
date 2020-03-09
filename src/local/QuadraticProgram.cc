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

#include "QuadraticProgram.h"

QuadraticProgram::QuadraticProgram(const std::string &experimentName)
: m_experimentName(experimentName)
, m_env(GRBEnv())
, m_model(m_env) {
}

void QuadraticProgram::printDebugFiles(std::ostream &out) {
    out << std::endl;
    out << "### Write the linear programm and the solution ###" << std::endl;

    std::string filename = m_experimentName + "/gurobi.lp";
    m_model.update();
    m_model.write(filename);
}

void QuadraticProgram::printResults(const std::string &filename) {
    std::ofstream file(m_experimentName + "/" + filename);
    if(!file.good()) {
        std::cerr << "QuadraticProgram::printResults(): open '" << m_experimentName << "/" << filename << "': failed" << std::endl;
        return;
    }

    printResults(file);
}

void QuadraticProgram::loadFirConfiguration(const std::string &filename, FirMethod method) {
    // Open the file
    std::ifstream file(filename, std::ios::binary);
    if (file.fail()) {
        std::cerr << "QuadraticProgram::loadFirConfiguration: The file '" << filename << "' is missing" << std::endl;
        std::exit(-1);
    }

    // Read the file until eof
    for (;;) {
        std::uint16_t nob = 0;
        std::uint16_t coeff = 0;
        double rejection = 0.0;

        // Read the values
        file.read(reinterpret_cast<char*>(&nob), sizeof(std::uint16_t));
        file.read(reinterpret_cast<char*>(&coeff), sizeof(std::uint16_t));
        file.read(reinterpret_cast<char*>(&rejection), sizeof(double));

        // If the end of file is reached
        if (file.eof()) {
            break;
        }

        // Add fir configuration
        m_firs.emplace_back(method, coeff, nob, rejection);
    }
}
