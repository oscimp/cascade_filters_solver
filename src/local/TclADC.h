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

#ifndef TCL_ADC_H
#define TCL_ADC_H

#include <fstream>

#include "TclProject.h"

/**
 * @brief Generate the tcl script for ADC source
 */
class TclADC: public TclProject {
private:
    /**
     * @brief Create the header of TCL script (PS7, ADC source...)
     *
     * @param file Output file
     * @param experimentName The experiment name
     */
    void writeTclHeader(std::ofstream &file, const std::string &experimentName) override;

    /**
     * @brief Add a filter stage
     *
     * @param file Output file
     * @param firNumber The stage number
     * @param filter The selected filter for the stage
     * @param previousSource The name of previous source. After the call it will be updated
     */
    virtual void addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, std::string &previousSource) override;

    /**
     * @brief Generate the footer of script (data ram blocks, run build...)
     *
     * @param file Output file
     * @param inputSize Size of input data
     * @param previousSource The last source
     * @param experimentName The name of experimentation
     */
    void writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &experimentName) override;
};

#endif // TCL_ADC_H
