#include "ScriptGenerator.h"

#include "LinearProgram.h"

void ScriptGenerator::generateDeployScript(const LinearProgram &lp, const std::string &outputFormat) {
    std::string scriptFilename = outputFormat + "/" + outputFormat + ".sh";

    std::ofstream file = createShellFile(scriptFilename);

    // Copy the write_bitstream
    safeShellCommand(file, "cp " + outputFormat + "/" + outputFormat + "_wrapper.bit /tmp/" + outputFormat + ".bit");

    // Create the the bif script
    file << "echo \"all:\" > /tmp/" + outputFormat + ".bif" << std::endl;
    file << "echo \"{\" >> /tmp/" + outputFormat + ".bif" << std::endl;
    file << "echo \"    /tmp/" + outputFormat + ".bit\" >> /tmp/" + outputFormat + ".bif" << std::endl;
    file << "echo \"}\" >> /tmp/" + outputFormat + ".bif" << std::endl;
    file << std::endl;

    // Generate the bit.bin file
    safeShellCommand(file, "source /opt/Xilinx/Vivado/2018.1/settings64.sh");
    safeShellCommand(file, "bootgen -image /tmp/" + outputFormat + ".bif -arch zynq -process_bitstream bin -w on");

    // Send the bitstream on the board
    safeShellCommand(file, "scp /tmp/" + outputFormat + ".bit.bin root@redpitaya:/lib/firmware");

    // Create the symlink to the bitstream
    safeShellCommand(file, "ssh root@redpitaya \"ln -sf /lib/firmware/" + outputFormat + ".bit.bin /lib/firmware/prn_symb.bit.bin\"");

    // Remove previous drivers
    safeShellCommand(file, "ssh root@redpitaya \"rmdir /sys/kernel/config/device-tree/overlays/prn/\"");
    safeShellCommand(file, "ssh root@redpitaya \"mkdir /sys/kernel/config/device-tree/overlays/prn/\"");

    // Select the right overlays
    auto selectedFilters = lp.getSelectedFilters();
    std::size_t nbStage = selectedFilters.size();
    safeShellCommand(file, "ssh root@redpitaya \"cat /usr/local/share/dtbo/chain-filter-" + std::to_string(nbStage) + ".dtbo > /sys/kernel/config/device-tree/overlays/prn/dtbo\"");

    // Configure the filters
    std::string filterList = "";
    for (auto filter: selectedFilters) {
        Fir fir = filter.filter;
        char filterName[128] = {0};
        switch (fir.getMethod()) {
        case FirMethod::Fir1:
            std::snprintf(filterName, 128, " /usr/local/share/filters/fir1/fir1_%03lu_int%02lu", fir.getCardC(), fir.getPiC());
            filterList += std::string(filterName);
            break;
        case FirMethod::FirLS:
            std::snprintf(filterName, 128, " /usr/local/share/filters/firls/firls_%03lu_int%02lu", fir.getCardC(), fir.getPiC());
            filterList += std::string(filterName);
            break;
        default:
            std::cerr << "ScriptGenerator::generateDeployScript: Kaiser filters no handled" << std::endl;
        }
    }
    safeShellCommand(file, "ssh root@redpitaya \"/usr/local/bin/prn_fir_loader_us" + filterList + "\"");

    // Get all the data
    safeShellCommand(file, "ssh root@redpitaya \"/usr/local/bin/prn_data_retreiver_us\"");
}

void ScriptGenerator::generateSimulationScript(const LinearProgram &lp, const std::string &outputFormat) {
    std::string scriptFilename = outputFormat + "/" + outputFormat + ".m";

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
    auto filters = lp.getSelectedFilters();
    for (const SelectedFilter &filter: filters) {
        file << "# Stage " << filter.stage << std::endl;

        // Création du nom du filtre
        Fir fir = filter.filter;
        char filterName[128] = {0};
        switch (fir.getMethod()) {
        case FirMethod::Fir1:
            std::snprintf(filterName, 128, "filters/fir1/fir1_%03lu_int%02lu", fir.getCardC(), fir.getPiC());
            break;
        case FirMethod::FirLS:
            std::snprintf(filterName, 128, "filters/firls/firls_%03lu_int%02lu", fir.getCardC(), fir.getPiC());
            break;
        default:
            std::cerr << "ScriptGenerator::generateSimulationScript: Kaiser filters no handled" << std::endl;
        }

        file << "b" << filter.stage << "= load(\"" << std::string(filterName) << "\");" << std::endl;
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
