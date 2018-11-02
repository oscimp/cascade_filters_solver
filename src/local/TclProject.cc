#include "TclProject.h"

#include <cmath>
#include <iostream>
#include <string>

#include "LinearProgram.h"


void TclProject::generate(const LinearProgram &lp, const std::string &outputFormat) {
    // Get FPGA_DEV path
    const char* env = std::getenv("OSC_IMP_DEV");
    if (env == nullptr || env[0] == 0) {
        std::cerr << "OSC_IMP_DEV variable not defined" << std::endl;
        std::exit(-1);
    }

    m_fpgaDevPath = std::string(env);

    // We generate the tcl file
    generateProjectFile(lp, outputFormat);
}

void TclProject::generateProjectFile(const LinearProgram &lp, const std::string &outputFormat) {
    // Create the file
    std::string scriptFilename = outputFormat + "/" + outputFormat + ".tcl";
    std::ofstream file(scriptFilename);

    // Write the tcl header
    writeTclHeader(file, outputFormat);

    // Add all firs
    auto filters = lp.getSelectedFilters();
    std::string previousSource = "$initial_source";
    int firNumber = 0;
    std::int64_t lastOutputSize = 0;

    for (const auto filter: filters) {
        lastOutputSize = addTclFir(file, firNumber, filter, previousSource);
        ++firNumber;
    }

    // Write the footer
    writeTclFooter(file, lastOutputSize, previousSource, outputFormat);
}

int TclProject::addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, std::string &previousSource) {
    Fir fir = filter.filter;
    std::string firName = "fir_" + std::to_string(firNumber);

    file << "# Create fir" << std::endl;
    file << "startgroup" << std::endl;
    file << "    # Create the block and configure it" << std::endl;
    file << "    set " << firName << " [ create_bd_cell -type ip -vlnv ggm:cogen:firReal:1.0 " << firName << " ]" << std::endl;
    file << "    set_property -dict [ list \\" << std::endl;
    file << "        CONFIG.NB_COEFF {" << std::to_string(fir.getCardC()) << "} \\" << std::endl;
    file << "        CONFIG.DECIMATE_FACTOR {1} \\" << std::endl;
    file << "        CONFIG.COEFF_SIZE {" << std::to_string(fir.getPiC()) << "} \\" << std::endl;
    file << "        CONFIG.DATA_IN_SIZE {" << std::to_string(filter.piIn) << "} \\" << std::endl;
    file << "        CONFIG.DATA_OUT_SIZE {" << std::to_string(filter.piIn + filter.piFir + 1) << "} ] $" << firName << std::endl;
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

    // if (filter.shift == 0) {
    //     previousSource = "$fir_" + std::to_string(firNumber) + "/data_out";
    //     return filter.piOut;
    // }

    std::string shifterName = "shifter_" + std::to_string(firNumber);
    file << "# Create shifter" << std::endl;
    file << "startgroup" << std::endl;
    file << "    # Create the block and configure it" << std::endl;
    file << "    set " << shifterName << " [ create_bd_cell -type ip -vlnv ggm:cogen:shifterReal:1.0 " << shifterName << " ]" << std::endl;
    file << "    set_property -dict [ list \\" << std::endl;
    file << "        CONFIG.DATA_OUT_SIZE {" << filter.piOut + 1 << "} \\" << std::endl;
    file << "        CONFIG.DATA_IN_SIZE {" << (filter.piIn + filter.piFir + 1) << "} ] $" << shifterName << std::endl;
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
    return filter.piOut + 1;
}
