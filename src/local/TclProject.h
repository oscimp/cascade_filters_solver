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

#ifndef TCL_PROJECT_H
#define TCL_PROJECT_H

#include <fstream>

class Fir;
class QuadraticProgram;
class SelectedFilter;

/**
 * @brief Encapuslate the TCL script generation
 */
class TclProject {
public:
    /**
     * @brief Generate the TCL script
     *
     * @param milp The quadratic program
     * @param experimentName The name of experimentation
     */
    void generate(const QuadraticProgram &milp, const std::string &experimentName);

protected:
    /**
     * @brief Create the TCL file
     *
     * @param milp The quadratic program
     * @param experimentName The experiment name
     */
    void generateProjectFile(const QuadraticProgram &milp, const std::string &experimentName);

    /**
     * @brief Create the header of TCL script (PS7, ADC source...)
     *
     * @param file Output file
     * @param experimentName The experiment name
     */
    virtual void writeTclHeader(std::ofstream &file, const std::string &experimentName) = 0;

    /**
     * @brief Add a filter stage
     *
     * @param file Output file
     * @param firNumber The stage number
     * @param filter The selected filter for the stage
     * @param previousSource The name of previous source. After the call it will be updated
     */
    virtual void addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, std::string &previousSource);

    /**
     * @brief Generate the footer of script (data ram blocks, run build...)
     *
     * @param file Output file
     * @param inputSize Size of input data
     * @param previousSource The last source
     * @param experimentName The name of experimentation
     */
    virtual void writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &experimentName) = 0;
};

#endif // TCL_PROJECT_H
