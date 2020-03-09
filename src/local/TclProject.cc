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

#include "TclProject.h"

#include <cmath>
#include <iostream>
#include <string>

#include "QuadraticProgram.h"


void TclProject::generate(const QuadraticProgram &milp, const std::string &experimentName) {
    // We generate the tcl file
    generateProjectFile(milp, experimentName);
}

void TclProject::generateProjectFile(const QuadraticProgram &milp, const std::string &experimentName) {
    // Create the file
    std::string scriptFilename = experimentName + "/" + experimentName + ".tcl";
    std::ofstream file(scriptFilename);

    // Write the tcl header
    writeTclHeader(file, experimentName);

    // Add all firs
    auto filters = milp.getSelectedFilters();
    std::string previousSource = "$initial_source";
    int firNumber = 0;

    for (const auto filter: filters) {
        addTclFir(file, firNumber, filter, previousSource);
        ++firNumber;
    }

    // The last output size are equal to the last pi_out + number of stage (=> 1 bit per stage)
    std::int64_t lastOutputSize = filters.back().piOut + filters.size();

    // Write the footer
    writeTclFooter(file, lastOutputSize, previousSource, experimentName);
}

void TclProject::addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, std::string &previousSource) {
    Fir fir = filter.filter;
    std::string firName = "fir_" + std::to_string(firNumber);

    file << "# Create fir" << std::endl;
    file << "startgroup" << std::endl;
    file << "    # Create the block and configure it" << std::endl;
    file << "    set " << firName << " [ create_bd_cell -type ip -vlnv ggm:cogen:firReal:1.0 " << firName << " ]" << std::endl;
    file << "    set_property -dict [ list \\" << std::endl;
    file << "        CONFIG.NB_COEFF {" << fir.getCardC() << "} \\" << std::endl;
    file << "        CONFIG.DECIMATE_FACTOR {1} \\" << std::endl;
    file << "        CONFIG.COEFF_SIZE {" << fir.getPiC() << "} \\" << std::endl;
    file << "        CONFIG.DATA_IN_SIZE {" << filter.piIn + firNumber << "} \\" << std::endl;
    file << "        CONFIG.DATA_OUT_SIZE {" << filter.piIn + fir.getPiFir() + firNumber + 1 << "} ] $" << firName << std::endl;
    file << std::endl;
    file << "    # Automation for AXI" << std::endl;
    file << "    apply_bd_automation -rule xilinx.com:bd_rule:axi4 \\" << std::endl;
    file << "       -config {Master \"/processing_system7_0/M_AXI_GP0\" Clk \"Auto\" } \\" << std::endl;
    file << "       [get_bd_intf_pins $" << firName << "/s00_axi]" << std::endl;
    file << std::endl;
    file << "    # Connect input data" << std::endl;
    file << "    connect_bd_intf_net\\" << std::endl;
    file << "       [get_bd_intf_pins " << previousSource << "] \\" << std::endl;
    file << "       [get_bd_intf_pins $" << firName << "/data_in]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;
    file << "# Save block design" << std::endl;
    file << "save_bd_design" << std::endl;
    file << std::endl;

    if (filter.shift == 0) {
        previousSource = "$"+ firName + "/data_out";
        return;
    }

    std::string shifterName = "shifter_" + std::to_string(firNumber);
    file << "# Create shifter" << std::endl;
    file << "startgroup" << std::endl;
    file << "    # Create the block and configure it" << std::endl;
    file << "    set " << shifterName << " [ create_bd_cell -type ip -vlnv ggm:cogen:shifterReal:1.0 " << shifterName << " ]" << std::endl;
    file << "    set_property -dict [ list \\" << std::endl;
    file << "        CONFIG.DATA_OUT_SIZE {" << filter.piOut + firNumber + 1 << "} \\" << std::endl;
    file << "        CONFIG.DATA_IN_SIZE {" << filter.piIn + fir.getPiFir() + firNumber + 1  << "} ] $" << shifterName << std::endl;
    file << std::endl;
    file << "    # Connect input data" << std::endl;
    file << "    connect_bd_intf_net\\" << std::endl;
    file << "       [get_bd_intf_pins $" << firName << "/data_out] \\" << std::endl;
    file << "       [get_bd_intf_pins $" << shifterName << "/data_in]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;
    file << "# Save block design" << std::endl;
    file << "save_bd_design" << std::endl;
    file << std::endl;

    previousSource = "$shifter_" + std::to_string(firNumber) + "/data_out";
}
