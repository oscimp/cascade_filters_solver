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

#include "TclADC.h"

#include <thread>

#include "QuadraticProgram.h"

void TclADC::writeTclHeader(std::ofstream &file, const std::string &experimentName) {
    file << "variable fpga_ip    $::env(OSC_IMP_IP)" << std::endl;
    file << "variable fpga_dev   $::env(OSC_IMP_DEV)" << std::endl;
    file << std::endl;
    file << "# Defines useful variable" << std::endl;

    // Part name for Redpitaya
    file << "set part_name xc7z010clg400-1" << std::endl;

    file << "set project_name " << experimentName << std::endl;
    file << "set bd_path /tmp/$project_name/$project_name.srcs/sources_1/bd/$project_name" << std::endl;
    file << std::endl;
    file << "# Remove old project" << std::endl;
    file << "file delete -force /tmp/$project_name" << std::endl;
    file << std::endl;
    file << "# Create the project" << std::endl;
    file << "create_project $project_name /tmp/$project_name -part $part_name" << std::endl;
    // file << "set_property BOARD_PART em.avnet.com:zed:part0:1.3 [current_project]" << std::endl;
    file << std::endl;
    file << "# create bd" << std::endl;
    file << "create_bd_design $project_name" << std::endl;
    file << std::endl;
    file << "# Add OSC_IMP IP" << std::endl;
    file << "set_property IP_REPO_PATHS $fpga_ip [current_project]" << std::endl;
    file << "update_ip_catalog" << std::endl;
    file << std::endl;

    file << "# Create PS7" << std::endl;
    file << "startgroup" << std::endl;
    file << "set preset_name $fpga_dev/redpitaya/redpitaya_preset.xml" << std::endl;
    file << "    # processing_system7_0, and set properties" << std::endl;
    file << "    set ps7 [ create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0 ]" << std::endl;
    file << "    set_property -dict [ list \\" << std::endl;
    file << "        CONFIG.PCW_IMPORT_BOARD_PRESET \"$fpga_ip/preset/redpitaya_preset.xml\" ] $ps7" << std::endl;
    file << std::endl;
    file << "    # Automation" << std::endl;
    file << "    apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 \\" << std::endl;
    file << "        -config {make_external \"FIXED_IO, DDR\" Master \"Disable\" Slave \"Disable\" } \\" << std::endl;
    file << "        $ps7" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Load RedPitaya ports" << std::endl;
    file << "source $fpga_ip/ltc2145/ltc2145.tcl" << std::endl;
    file << "source $fpga_ip/redpitaya_adc_dac_clk/redpitaya_adc_dac_clk.tcl" << std::endl;
    file << std::endl;

    file << "# Create ADC-DAC clock" << std::endl;
    file << "startgroup" << std::endl;
    file << "    # Create the block and configure it" << std::endl;
    file << "    set clocks [ create_bd_cell -type ip \\" << std::endl;
    file << "    	-vlnv ggm:cogen:redpitaya_adc_dac_clk:1.0 redpitaya_adc_dac_clk_0 ]" << std::endl;
    file << std::endl;
    file << "    # Connect external ports" << std::endl;
    file << "    connect_bd_net -net adc_clk_n_i_0 [get_bd_ports adc_clk_n_i] \\" << std::endl;
    file << "    	[get_bd_pins $clocks/adc_clk_n_i]" << std::endl;
    file << "    connect_bd_net -net adc_clk_p_i_0 [get_bd_ports adc_clk_p_i] \\" << std::endl;
    file << "    	[get_bd_pins $clocks/adc_clk_p_i]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Create ADC block" << std::endl;
    file << "startgroup" << std::endl;
    file << "    # Create the block and configure it" << std::endl;
    file << "    set adc [ create_bd_cell -type ip -vlnv gwbs:user:ltc2145:1.0 ltc2145_0 ]" << std::endl;
    file << "" << std::endl;
    file << "    # Connect external ports" << std::endl;
    file << "    connect_bd_net -net adc_data_a_i_0 [get_bd_ports adc_data_a_i] \\" << std::endl;
    file << "        [get_bd_pins $adc/adc_data_a_i]" << std::endl;
    file << "    connect_bd_net -net adc_data_b_i_0 [get_bd_ports adc_data_b_i] \\" << std::endl;
    file << "        [get_bd_pins $adc/adc_data_b_i]" << std::endl;
    file << "    connect_bd_net -net adc_cdcs_0 [get_bd_ports adc_cdcs] \\" << std::endl;
    file << "        [get_bd_pins $adc/adc_cdcs]" << std::endl;
    file << std::endl;
    file << "    # Connect clock and rest signals" << std::endl;
    file << "    connect_bd_net \\" << std::endl;
    file << "    	[get_bd_pins $clocks/adc_clk_o] \\" << std::endl;
    file << "    	[get_bd_pins $adc/adc_clk_i]" << std::endl;
    file << "    connect_bd_net \\" << std::endl;
    file << "    	[get_bd_pins $clocks/adc_rstn_o] \\" << std::endl;
    file << "    	[get_bd_pins $adc/resetn]" << std::endl;
    file << "    connect_bd_net \\" << std::endl;
    file << "    	[get_bd_pins $clocks/adc_rst_o] \\" << std::endl;
    file << "    	[get_bd_pins $adc/processing_rst_i]" << std::endl;
    file << "endgroup" << std::endl;

    file << "# Add constraint" << std::endl;
    file << "add_files -norecurse -fileset constrs_1 $fpga_ip/ltc2145/ltc2145-redpy.xdc" << std::endl;
    file << "add_files -norecurse -fileset constrs_1 $fpga_ip/ltc2145/ltc2145_ooc.xdc" << std::endl;
    file << "add_files -norecurse -fileset constrs_1 $fpga_ip/redpitaya_adc_dac_clk/redpitaya_clk_pin.xdc" << std::endl;
    file << std::endl;

    file << "# Create splitter to raw data and processed data" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set splitter_raw_data [ create_bd_cell -type ip -vlnv ggm:cogen:dupplReal_1_to_2:1.0 dupplReal_1_to_2_0 ]" << std::endl;
    file << "   set_property -dict [list CONFIG.DATA_SIZE {14}] $splitter_raw_data" << std::endl;
    file << "   connect_bd_intf_net [get_bd_intf_pins $splitter_raw_data/data_in] [get_bd_intf_pins $adc/data_a]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Create ADC expander for proccessed data" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set shifter_adc_proc_data [ create_bd_cell -type ip -vlnv ggm:cogen:expanderReal:1.0 expanderReal_0 ]" << std::endl;
    file << "   set_property -dict [list CONFIG.DATA_IN_SIZE {14} CONFIG.DATA_OUT_SIZE {16}] $shifter_adc_proc_data" << std::endl;
    file << "   connect_bd_intf_net [get_bd_intf_pins $shifter_adc_proc_data/data_in] [get_bd_intf_pins $splitter_raw_data/data1_out]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Create ADC expander for raw data" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set expander_adc_raw_data [ create_bd_cell -type ip -vlnv ggm:cogen:expanderReal:1.0 expanderReal_1 ]" << std::endl;
    file << "   set_property -dict [list CONFIG.DATA_IN_SIZE {14} CONFIG.DATA_OUT_SIZE {64}] $expander_adc_raw_data" << std::endl;
    file << "   connect_bd_intf_net [get_bd_intf_pins $expander_adc_raw_data/data_in] [get_bd_intf_pins $splitter_raw_data/data2_out]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Save block design" << std::endl;
    file << "save_bd_design" << std::endl;
    file << std::endl;

    file << "# Set the initial source" << std::endl;
    file << "set initial_source $shifter_adc_proc_data/data_out" << std::endl;
    file << std::endl;
}

void TclADC::addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, std::string &previousSource) {
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
    if (firNumber == 0) {
        file << "        CONFIG.DATA_IN_SIZE {16} \\" << std::endl;
    }
    else {
        file << "        CONFIG.DATA_IN_SIZE {" << filter.piIn << "} \\" << std::endl;
    }
    file << "        CONFIG.DATA_OUT_SIZE {" << (filter.piIn + fir.getPiFir()) << "} ] $" << firName << std::endl;
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

    std::string shifterName = "shifter_" + std::to_string(firNumber);
    file << "# Create shifter" << std::endl;
    file << "startgroup" << std::endl;
    file << "    # Create the block and configure it" << std::endl;
    file << "    set " << shifterName << " [ create_bd_cell -type ip -vlnv ggm:cogen:shifterReal:1.0 " << shifterName << " ]" << std::endl;
    file << "    set_property -dict [ list \\" << std::endl;
    file << "        CONFIG.DATA_OUT_SIZE {" << filter.piOut << "} \\" << std::endl;
    file << "        CONFIG.DATA_IN_SIZE {" << (filter.piIn + fir.getPiFir()) << "} ] $" << shifterName << std::endl;
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

void TclADC::writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &experimentName) {
    file << "# Connect reset" << std::endl;
    file << "connect_bd_net [get_bd_pins $clocks/adc_rst_i] \\" << std::endl;
    file << "    [get_bd_pins rst_ps7_0_125M/peripheral_reset]" << std::endl;
    file << std::endl;

    if (inputSize > 64) {
        file << "# Create Ram shifter block" << std::endl;
        file << "startgroup" << std::endl;
        file << "   set shifter_ram [create_bd_cell -type ip -vlnv ggm:cogen:shifterReal:1.0 shifterReal_0]" << std::endl;
        file << "   set_property -dict [list CONFIG.DATA_OUT_SIZE {64} CONFIG.DATA_IN_SIZE {" << std::to_string(inputSize) << "}] $shifter_ram" << std::endl;
        file << "   connect_bd_intf_net [get_bd_intf_pins " << previousSource << "] [get_bd_intf_pins $shifter_ram/data_in]" << std::endl;
        file << "endgroup" << std::endl;
        file << std::endl;

        previousSource = "$shifter_ram/data_out";
    }
    else if (inputSize < 64) {
        file << "# Create Ram expander block" << std::endl;
        file << "startgroup" << std::endl;
        file << "   set expander_ram [create_bd_cell -type ip -vlnv ggm:cogen:expanderReal:1.0 expanderReal_2]" << std::endl;
        file << "   set_property -dict [list CONFIG.DATA_IN_SIZE {" << std::to_string(inputSize) << "} CONFIG.DATA_OUT_SIZE {64}] $expander_ram" << std::endl;
        file << "   connect_bd_intf_net [get_bd_intf_pins " << previousSource << "] [get_bd_intf_pins $expander_ram/data_in]" << std::endl;
        file << "endgroup" << std::endl;
        file << std::endl;

        previousSource = "$expander_ram/data_out";
    }

    file << "# Create Ram block for processed data" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set ram_fir [create_bd_cell -type ip -vlnv ggm:cogen:data64_1_voie_to_ram:1.0 data64_1_voie_to_ram_0]" << std::endl;
    file << "   apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { \\" << std::endl;
    file << "       Clk_master {/processing_system7_0/FCLK_CLK0 (125 MHz)} Clk_slave {Auto}\\" << std::endl;
    file << "       Clk_xbar {/processing_system7_0/FCLK_CLK0 (125 MHz)}\\" << std::endl;
    file << "       Master {/processing_system7_0/M_AXI_GP0} Slave {/data64_1_voie_to_ram_0/s00_axi}\\" << std::endl;
    file << "       intc_ip {/ps7_0_axi_periph} master_apm {0}}  [get_bd_intf_pins $ram_fir/s00_axi]" << std::endl;
    file << "   connect_bd_intf_net [get_bd_intf_pins $ram_fir/data_in] [get_bd_intf_pins " << previousSource << "]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Create Ram block for raw data" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set ram_prn [create_bd_cell -type ip -vlnv ggm:cogen:data64_1_voie_to_ram:1.0 data64_1_voie_to_ram_1]" << std::endl;
    file << "   apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { \\" << std::endl;
    file << "       Clk_master {/processing_system7_0/FCLK_CLK0 (125 MHz)} Clk_slave {Auto}\\" << std::endl;
    file << "       Clk_xbar {/processing_system7_0/FCLK_CLK0 (125 MHz)}\\" << std::endl;
    file << "       Master {/processing_system7_0/M_AXI_GP0} Slave {/data64_1_voie_to_ram_1/s00_axi}\\" << std::endl;
    file << "       intc_ip {/ps7_0_axi_periph} master_apm {0}}  [get_bd_intf_pins $ram_prn/s00_axi]" << std::endl;
    file << "   connect_bd_intf_net [get_bd_intf_pins $ram_prn/data_in] [get_bd_intf_pins $expander_adc_raw_data/data_out]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Save block design" << std::endl;
    file << "save_bd_design" << std::endl;
    file << std::endl;

    file << "# Generate the block design" << std::endl;
    file << "generate_target all [get_files  $bd_path/$project_name.bd]" << std::endl;
    file << "make_wrapper -files [get_files $bd_path/$project_name.bd] -top" << std::endl;
    file << std::endl;
    file << "# Set the top block" << std::endl;
    file << "set project_name_wrapper $project_name" << std::endl;
    file << "append project_name_wrapper _wrapper" << std::endl;
    file << "add_files -norecurse $bd_path/hdl/$project_name_wrapper.v" << std::endl;
    file << std::endl;
    file << "# Load any additional Verilog files in the project folder" << std::endl;
    file << "set files [glob -nocomplain projects/$project_name/*.v projects/$project_name/*.sv]" << std::endl;
    file << "if {[llength $files] > 0} {" << std::endl;
    file << "  add_files -norecurse $files" << std::endl;
    file << "}" << std::endl;
    file << std::endl;

    // Get the number of threads
    unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
    if (concurentThreadsSupported == 0) {
        concurentThreadsSupported = 1;
    }

    file << "# Create bitstream" << std::endl;
    file << "launch_runs synth_1 -jobs " << std::to_string(concurentThreadsSupported) << std::endl;
    file << "wait_on_run synth_1" << std::endl;
    file << "launch_runs impl_1 -to_step write_bitstream -jobs " << std::to_string(concurentThreadsSupported) << std::endl;
    file << "wait_on_run impl_1" << std::endl;
    file << std::endl;

    file << "# export usage" << std::endl;
    file << "open_run impl_1" << std::endl;
    file << "report_utilization -hierarchical -hierarchical_depth 1 -file " << experimentName << "/" << experimentName << "_usage_ressources.txt" << std::endl;
    file << std::endl;

    file << "# Copy the bitstream" << std::endl;
    file << "file copy -force /tmp/" << experimentName << "/" << experimentName << ".runs/impl_1/" << experimentName << "_wrapper.bit " << experimentName << "/" << experimentName << "_wrapper.bit" << std::endl;
    file << "exit" << std::endl;
}
