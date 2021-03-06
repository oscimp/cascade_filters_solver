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

#ifndef MINIMIZE_AREA_H
#define MINIMIZE_AREA_H

#include "QuadraticProgram.h"

/**
 * @brief Quadratic program to minimize area
 *
 * @see QuadraticProgram
 */
class MinimizeArea: public QuadraticProgram {
public:
    using QuadraticProgram::printResults;

    /**
     * @brief Constructor
     *
     * @param nbStage Total stage
     * @param rejectionLevel Rejection constraint
     * @param firlsFile Path to firls filters
     * @param fir1File Path to fir1 filters
     * @param experimentName Name of experiment
     */
    MinimizeArea(const std::int64_t nbStage, const double rejectionLevel, const std::string &jsonPath, const std::string &experimentName);

    /**
     * @brief Get the optimal selected filters
     */
    const std::vector<SelectedFilter> &getSelectedFilters() const override;

    /**
     * @brief Print the results to output stream
     *
     * @param out Output stream
     */
    void printResults(std::ostream &out = std::cout) override;

private:
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
};

#endif // MINIMIZE_AREA_H
