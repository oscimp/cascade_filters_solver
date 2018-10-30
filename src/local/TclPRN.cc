#include "TclPRN.h"

#include <thread>

void TclPRN::writeTclHeader(std::ofstream &file, const std::string &outputFormat) {
    file << "variable fpga_ip    $::env(OSC_IMP_IP)" << std::endl;
    file << "variable fpga_dev   $::env(OSC_IMP_DEV)" << std::endl;
    file << std::endl;
    file << "# Defines useful variable" << std::endl;

    // Part name for Redpitaya
    file << "set part_name xc7z010clg400-1" << std::endl;

    file << "set project_name " << outputFormat << std::endl;
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
    file << "        CONFIG.PCW_IMPORT_BOARD_PRESET {" << m_fpgaDevPath << "/redpitaya/redpitaya_preset.xml}] $ps7" << std::endl;
    // file << "        CONFIG.PRESET {ZedBoard} \\" << std::endl;
    // file << "        CONFIG.PCW_FPGA1_PERIPHERAL_FREQMHZ {200.000000} \\" << std::endl;
    // file << "        CONFIG.PCW_EN_CLK1_PORT {1} ] $processing_system7_0" << std::endl;
    file << std::endl;
    file << "    # Automation" << std::endl;
    file << "    apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 \\" << std::endl;
    file << "        -config {make_external \"FIXED_IO, DDR\" Master \"Disable\" Slave \"Disable\" } \\" << std::endl;
    file << "        $ps7" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;
    file << "# Save block design" << std::endl;
    file << "save_bd_design" << std::endl;
    file << std::endl;

    file << "# Create PNR block" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set prn [ create_bd_cell -type ip -vlnv ggm:cogen:prn20b:1.0 prn20b_0 ]" << std::endl;
    file << "   apply_bd_automation -rule xilinx.com:bd_rule:axi4 \\" << std::endl;
    file << "       -config { Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} \\" << std::endl;
    file << "       Master {/processing_system7_0/M_AXI_GP0} Slave {/prn20b_0/s00_axi} intc_ip {New AXI Interconnect} master_apm {0}}  [get_bd_intf_pins $prn/s00_axi]" << std::endl;
    file << "   connect_bd_net [get_bd_pins $prn/ref_clk_i] [get_bd_pins $ps7/FCLK_CLK0]" << std::endl;
    file << "   connect_bd_net [get_bd_pins $prn/ref_rst_i] [get_bd_pins rst_ps7_0_125M/peripheral_reset]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Create PNR shifter for the PRN data" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set shifter_prn_data [ create_bd_cell -type ip -vlnv ggm:cogen:shifterReal:1.0 shifterReal_0 ]" << std::endl;
    file << "   set_property -dict [list CONFIG.DATA_OUT_SIZE {16} CONFIG.DATA_IN_SIZE {20}] $shifter_prn_data" << std::endl;
    file << "   connect_bd_intf_net [get_bd_intf_pins $prn/prn_full_out] [get_bd_intf_pins $shifter_prn_data/data_in]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Create splitter to raw data and processed data" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set splitter_raw_data [ create_bd_cell -type ip -vlnv ggm:cogen:dupplReal_1_to_2:1.0 dupplReal_1_to_2_0 ]" << std::endl;
    file << "   set_property -dict [list CONFIG.DATA_SIZE {16}] $splitter_raw_data" << std::endl;
    file << "   connect_bd_intf_net [get_bd_intf_pins $shifter_prn_data/data_out] [get_bd_intf_pins $splitter_raw_data/data_in]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Create PNR expander for raw data" << std::endl;
    file << "startgroup" << std::endl;
    file << "   set expander_prn_raw_data [ create_bd_cell -type ip -vlnv ggm:cogen:expanderReal:1.0 expanderReal_0 ]" << std::endl;
    file << "   set_property -dict [list CONFIG.DATA_IN_SIZE {16} CONFIG.DATA_OUT_SIZE {64}] $expander_prn_raw_data" << std::endl;
    file << "   connect_bd_intf_net [get_bd_intf_pins $splitter_raw_data/data2_out] [get_bd_intf_pins $expander_prn_raw_data/data_in]" << std::endl;
    file << "endgroup" << std::endl;
    file << std::endl;

    file << "# Save block design" << std::endl;
    file << "save_bd_design" << std::endl;
    file << std::endl;

    file << "# Set the initial source" << std::endl;
    file << "set initial_source $splitter_raw_data/data1_out" << std::endl;
    file << std::endl;
}

void TclPRN::writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &outputFormat) {
    if (inputSize > 64) {
        file << "# Create Ram shifter block" << std::endl;
        file << "startgroup" << std::endl;
        file << "   set shifter_ram [create_bd_cell -type ip -vlnv ggm:cogen:shifterReal:1.0 shifterReal_1]" << std::endl;
        file << "   set_property -dict [list CONFIG.DATA_OUT_SIZE {64} CONFIG.DATA_IN_SIZE {" << std::to_string(inputSize) << "}] $shifter_ram" << std::endl;
        file << "   connect_bd_intf_net [get_bd_intf_pins " << previousSource << "] [get_bd_intf_pins $shifter_ram/data_in]" << std::endl;
        file << "endgroup" << std::endl;
        file << std::endl;

        previousSource = "$shifter_ram/data_out";
    }
    else if (inputSize < 64) {
        file << "# Create Ram expander block" << std::endl;
        file << "startgroup" << std::endl;
        file << "   set expander_ram [create_bd_cell -type ip -vlnv ggm:cogen:expanderReal:1.0 expanderReal_1]" << std::endl;
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
    file << "   connect_bd_intf_net [get_bd_intf_pins $ram_prn/data_in] [get_bd_intf_pins $expander_prn_raw_data/data_out]" << std::endl;
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
    file << "report_utilization -hierarchical -hierarchical_depth 1 -file " << outputFormat << "/" << outputFormat << "_usage_ressources.txt" << std::endl;
    file << std::endl;

    file << "# Copy the bitstream" << std::endl;
    file << "file copy -force /tmp/" << outputFormat << "/" << outputFormat << ".runs/impl_1/" << outputFormat << "_wrapper.bit " << outputFormat << "/" << outputFormat << "_wrapper.bit" << std::endl;
    file << "exit" << std::endl;
}
