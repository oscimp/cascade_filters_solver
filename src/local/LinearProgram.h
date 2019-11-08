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

#ifndef LINEAR_PROGRAM_H
#define LINEAR_PROGRAM_H

#include <ostream>
#include <string>
#include <vector>

#include <gurobi_c++.h>

#include "Fir.h"

struct SelectedFilter {
    std::int64_t stage;
    Fir filter;
    double rejection;
    std::int64_t shift;
    std::int64_t piIn;
    std::int64_t piFir;
    std::int64_t piOut;
};

class LinearProgram {
public:
    LinearProgram(const std::int64_t nbStage, const double areaMax, const std::string &firlsFile, const std::string &fir1File, const std::string &outputFormat);

    const std::vector<SelectedFilter> &getSelectedFilters() const;

    void printDebugFile();
    void printResults();
    void printResults(const std::string &filename);

private:
    /*! \brief Load FIR confiuration form binary file
     * The format of binary file is :
     * 1) uint16 (number of coefficients)
     * 2) uint16 (number of bit for each coefficient)
     * 2) double (noise rejection)
     *
     * \param filename Binary filename
     * \param method Algorithm used to create the coefficients
     */
    void loadFirConfiguration(const std::string &filename, FirMethod method);

    void printResults(std::ostream &out);

private:
    std::vector<Fir> m_firs;

    GRBEnv m_env;
    GRBModel m_model;
    std::vector< std::vector<GRBVar> > m_var_delta;
    std::vector< std::vector<GRBVar> > m_var_pi_fir;
    std::vector<GRBVar> m_var_pi_s;
    std::vector<GRBVar> m_var_a;
    std::vector<GRBVar> m_var_r;
    std::vector<GRBVar> m_var_pi;
    GRBVar m_var_PI_IN;

    std::vector<SelectedFilter> m_selectedFilters;
    double m_areaValue;
    double m_rejectionValue;
    double m_lastPi;
    double m_computationTime;

    std::string m_outputFormat;
};

#endif // LINEAR_PROGRAM_H
