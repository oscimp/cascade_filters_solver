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

class TclProject {
public:
    void generate(const QuadraticProgram &milp, const std::string &experimentName);

protected:
    void generateProjectFile(const QuadraticProgram &milp, const std::string &experimentName);
    virtual void writeTclHeader(std::ofstream &file, const std::string &experimentName) = 0;
    virtual void addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, std::string &previousSource);
    virtual void writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &experimentName) = 0;
};

#endif // TCL_PROJECT_H
