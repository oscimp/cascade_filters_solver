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

#include "ScriptGenerator.h"

#include "QuadraticProgram.h"

void ScriptGenerator::generateDeployScript(const QuadraticProgram &milp, const std::string &experimentName, const std::string dtboType) {
    std::string scriptFilename = experimentName + "/" + experimentName + ".sh";

    std::ofstream file = createShellFile(scriptFilename);

    // Copy the write_bitstream
    safeShellCommand(file, "cp " + experimentName + "/" + experimentName + "_wrapper.bit /tmp/" + experimentName + ".bit");

    // Create the the bif script
    file << "echo \"all:\" > /tmp/" + experimentName + ".bif" << std::endl;
    file << "echo \"{\" >> /tmp/" + experimentName + ".bif" << std::endl;
    file << "echo \"    /tmp/" + experimentName + ".bit\" >> /tmp/" + experimentName + ".bif" << std::endl;
    file << "echo \"}\" >> /tmp/" + experimentName + ".bif" << std::endl;
    file << std::endl;

    // Generate the bit.bin file
    safeShellCommand(file, "source /opt/Xilinx/Vivado/2018.1/settings64.sh");
    safeShellCommand(file, "bootgen -image /tmp/" + experimentName + ".bif -arch zynq -process_bitstream bin -w on");

    // Send the bitstream on the board
    safeShellCommand(file, "scp /tmp/" + experimentName + ".bit.bin root@redpitaya:/lib/firmware");

    // Create the symlink to the bitstream
    safeShellCommand(file, "ssh root@redpitaya \"ln -sf /lib/firmware/" + experimentName + ".bit.bin /lib/firmware/prn_symb.bit.bin\"");

    // Remove previous drivers
    safeShellCommand(file, "ssh root@redpitaya \"rmdir /sys/kernel/config/device-tree/overlays/prn/\"");
    safeShellCommand(file, "ssh root@redpitaya \"mkdir /sys/kernel/config/device-tree/overlays/prn/\"");

    // Select the right overlays
    auto selectedFilters = milp.getSelectedFilters();
    std::size_t nbStage = selectedFilters.size();
    safeShellCommand(file, "ssh root@redpitaya \"cat /usr/local/share/dtbo/" + dtboType +"/chain-filter-" + std::to_string(nbStage) + ".dtbo > /sys/kernel/config/device-tree/overlays/prn/dtbo; sleep 1\"");

    // Configure the filters
    std::string filterList = "";
    for (auto filter: selectedFilters) {
        const Fir &fir = filter.filter;
        filterList += "/usr/local/share/" + fir.getFilterName() + " ";
    }
    safeShellCommand(file, "ssh root@redpitaya \"/usr/local/bin/prn_fir_loader_us " + filterList + "\"");

    // Generate all the data
    safeShellCommand(file, "ssh root@redpitaya \"/usr/local/bin/prn_data_retreiver_us\"");

    // Retrive data
    safeShellCommand(file, "scp root@redpitaya:~/data_prn.bin " + experimentName + "/data_" + dtboType + ".bin");
    safeShellCommand(file, "scp root@redpitaya:~/data_fir.bin " + experimentName + "/data_" + std::to_string(nbStage) + "_fir.bin");
}

void ScriptGenerator::generateSimulationScript(const QuadraticProgram &milp, const std::string &experimentName) {
    std::string scriptFilename = experimentName + "/" + experimentName + ".m";

    std::ofstream file = createOctaveFile(scriptFilename);

    // Définition des constantes
    file << "N = 2048;" << std::endl;
    file << "PiIn = 16;" << std::endl;
    file << std::endl;

    // Génération du résultat avec un bruit blanc
    file << "## Avec noise" << std::endl;
    file << "# Génération du bruit" << std::endl;
    file << "noise = rand(1, N);" << std::endl;
    file << "noise = noise - mean(noise);" << std::endl;
    file << "noise_norm = noise ./ max(abs(noise));" << std::endl;
    file << "noise_int = noise_norm .* (2^(PiIn - 1) - 1);" << std::endl;
    file << std::endl;

    // Création des filtres
    std::string previousSource = "noise_int";
    auto filters = milp.getSelectedFilters();
    for (const SelectedFilter &filter: filters) {
        file << "# Stage " << filter.stage << std::endl;
        file << "b" << filter.stage << "= load(\"" << filter.filter.getFilterName() << "\");" << std::endl;
        file << "fir_data" << filter.stage << " = filter(b" << filter.stage << ", 1, " << previousSource << ");" << std::endl;
        file << "shift_data" << filter.stage << " = fir_data" << filter.stage << " ./ (2^" << filter.shift << ");" << std::endl;
        file << std::endl;

        previousSource = "shift_data" + std::to_string(filter.stage);
    }

    // Création du spectrum
    file << "spectrum = abs(fft(hanning(N)' .* " << previousSource << ")(1:N/2));" << std::endl;
    file << "log_data = 20*log10(spectrum ./ mean(spectrum(50:200)));" << std::endl;

    // Creation de la référence
    file << "## Avec freqz" << std::endl;
    file << "hTotal = ones(N/2, 1);" << std::endl;
    file << std::endl;

    // Creation des étages
    for (const SelectedFilter &filter: filters) {
        file << "# Stage " << filter.stage << std::endl;
        file << "[h, w] = freqz(b" << filter.stage << ", 1, N/2);" << std::endl;
        file << "hTotal = hTotal .* h;" << std::endl;
        file << std::endl;
    }

    // Normalisation de la référence
    file << "mag = abs(hTotal);" << std::endl;
    file << "mag = mag ./ mag(1);" << std::endl;
    file << "log_freqz = 20 * log10(mag);" << std::endl;
    file << "write_binary(\"freqz_" << filters.size() << "_stage.bin\", log_freqz);" << std::endl;
    file << std::endl;

    // Affichage des résultats
    file << "clf;" << std::endl;
    file << "f_axe = [1:N/2] * 1/N;" << std::endl;
    file << "hold on;" << std::endl;
    file << "plot(f_axe, log_freqz);" << std::endl;
    file << "plot(f_axe, log_data);" << std::endl;
    file << "hold off;" << std::endl;
    file << std::endl;

    // Légende du graphique
    file << "title(\"Simulation of cascade filters with shift\")" << std::endl;
    file << "ylabel(\"Rejection (dB)\")" << std::endl;
    file << "xlabel(\"Normalize frequency (half of nyquist)\")" << std::endl;
    file << "legend(\"Perfect filter (with freqz)\", \"Filter with 16 bits input noise (octave)\")" << std::endl;
    file << std::endl;
}

std::ofstream ScriptGenerator::createShellFile(const std::string &scriptFilename) {
    std::ofstream file(scriptFilename);

    if (!file.good()) {
        std::cerr << "ScriptGenerator::createShellFile: Open " << scriptFilename << " file: failed" << std::endl;
        std::exit(1);
    }

    file << "#!/bin/bash" << std::endl;
    file << std::endl;

    return file;
}

void ScriptGenerator::safeShellCommand(std::ofstream &file, const std::string &command, int expectedReturn) {
    file << command << std::endl;
    file << "if [ $? -ne " << expectedReturn << " ]" << std::endl;
    file << "then" << std::endl;
    file << "    echo \"Command '" << command << "': failed\"" << std::endl;
    file << "    exit 1" << std::endl;
    file << "fi" << std::endl;
    file << std::endl;
}

std::ofstream ScriptGenerator::createOctaveFile(const std::string &scriptFilename) {
    std::ofstream file(scriptFilename);

    if (!file.good()) {
        std::cerr << "ScriptGenerator::createOctaveFile: Open " << scriptFilename << " file: failed" << std::endl;
        std::exit(1);
    }

    file << "clear all;" << std::endl;
    file << "close all;" << std::endl;
    file << std::endl;

    return file;
}
