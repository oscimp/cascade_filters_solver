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

#ifndef SCRIPT_GENERATOR
#define SCRIPT_GENERATOR

#include <iostream>

class QuadraticProgram;

/**
 * @brief Utility class to generate scripts
 */
class ScriptGenerator {
public:
    /**
     * @brief Generate the shell script to deploy the design on FPGA board
     *
     * @param milp The quadratic program
     * @param experimentName The name of experimentation
     * @param dtboType The name of dtbo
     */
    static void generateDeployScript(const QuadraticProgram &milp, const std::string &experimentName, const std::string dtboType);

    /**
     * @brief Generate the octave simulation script
     *
     * @param milp The quadratic program
     * @param experimentName The name of experimentation
     */
    static void generateSimulationScript(const QuadraticProgram &milp, const std::string &experimentName);

private:
    static std::ofstream createShellFile(const std::string &scriptFilename);
    static void safeShellCommand(std::ofstream &file, const std::string &command, int expectedReturn = 0);
    static std::ofstream createOctaveFile(const std::string &scriptFilename);
};

#endif // SCRIPT_GENERATOR
