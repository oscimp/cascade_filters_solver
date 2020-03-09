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

#ifndef QUADRATIC_PROGRAM_H
#define QUADRATIC_PROGRAM_H

#include <cinttypes>
#include <vector>

#include <gurobi_c++.h>

#include "Fir.h"

/**
 * @brief Structure to handle the filter selection
 *
 * @see Fir
 */
struct SelectedFilter {
    std::int64_t stage;     /*!< Indicate the stage  */
    Fir filter;             /*!< Describe the selected filter */
    double rejection;       /*!< Indicate the total rejection */
    std::int64_t shift;     /*!< Indicate the number of shited bits */
    std::int64_t piIn;      /*!< Indicate the number of input bits */
    std::int64_t piFir;     /*!< Indicate the number of bits added by the filter */
    std::int64_t piOut;     /*!< Indicate the number of output bits */
};

/**
 * @brief Interface of any instances of quadratic model
 *
 * @see MaximizeRejection
 * @see MinimizeArea
 */
class QuadraticProgram {
public:
    /**
     * @brief Constructor
     *
     * @param experimentName Name used to create some folders and files
     */
    QuadraticProgram(const std::string &experimentName);

    /**
     * @brief Default destructor
     */
    virtual ~QuadraticProgram() {}

    /**
     * @brief Get the selected filters
     */
    virtual const std::vector<SelectedFilter>& getSelectedFilters() const = 0;

    /**
     * @brief Print the debug files
     *
     * @param out Output stream
     */
    virtual void printDebugFiles(std::ostream &out = std::cout);

    /**
     * @brief Print the result files
     *
     * @param out Output stream
     */
    virtual void printResults(std::ostream &out = std::cout) = 0;

    /**
     * @brief Print the result into indicate file
     *
     * @param filename Path of output file
     */
    void printResults(const std::string &filename);

protected:
    /**
     * @brief Load FIR confiuration form binary file
     * The format of binary file is :
     * 1) uint16 (number of coefficients)
     * 2) uint16 (number of bit for each coefficient)
     * 3) double (noise rejection)
     *
     * @param filename Binary filename
     * @param method Algorithm used to create the coefficients
     */
    void loadFirConfiguration(const std::string &filename, FirMethod method);

protected:
    const std::string m_experimentName;   /*!< Experiment name used to create directory and files */
    GRBEnv m_env;                         /*!< Gurobi environnement */
    GRBModel m_model;                     /*!< Gurobi model */

    std::vector<Fir> m_firs;              /*!< Storage for all filter configurations */
};

#endif // QUADRATIC_PROGRAM_H
