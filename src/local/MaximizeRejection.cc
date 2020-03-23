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

#include "MaximizeRejection.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

MaximizeRejection::MaximizeRejection(const std::int64_t nbStage, const double areaMax, const std::string &jsonPath, const std::string &experimentName)
: QuadraticProgram(experimentName)
, m_areaValue(0.0)
, m_rejectionValue(0.0)
, m_lastPi(0.0)
, m_computationTime(0.0) {
    // Read JSON file to get the filters file loctations
    std::ifstream jsonFile(jsonPath, std::ios::binary);
    if (jsonFile.fail()) {
        std::cerr << "MaximizeRejection::MaximizeRejection: The json file '" << jsonPath << "' is missing" << std::endl;
        std::exit(-1);
    }

    // Get relative path
    fs::path filtersDirectory = fs::path(jsonPath).parent_path();

    // Create an object json
    json jsonData;
    jsonFile >> jsonData;

    // Add all filters
    for (auto& element : jsonData.items()) {
        std::string filterPath(filtersDirectory.string() + "/" + static_cast<std::string>(element.value()));
        loadFirConfiguration(filterPath, element.key());
    }

    std::cout << "Total config FIR: " << m_firs.size() << std::endl;

    // Déclaration des constantes internes
    const std::int64_t NbConfFir = m_firs.size();
    const std::int64_t NbStage = nbStage;
    const std::int64_t PiIn = 16; // PRN input
    // const std::int64_t PiIn = 7; // ADC input
    const std::int64_t PiMax = 256;
    const double AMax = areaMax;

    // Déclaration des variables delta
    m_var_delta.resize(NbStage);
    for (std::int64_t i = 0; i < NbStage; ++i) {
        m_var_delta[i].resize(NbConfFir);
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string varName = "delta_" + std::to_string(i) + "_" + std::to_string(j);
            m_var_delta[i][j] = m_model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
        }
    }

    // Déclaration des pi_fir
    m_var_pi_fir.resize(NbStage);
    for (std::int64_t i = 0; i < NbStage; ++i) {
        m_var_pi_fir[i].resize(NbConfFir);
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string varName = "pi_fir_" + std::to_string(i) + "_" + std::to_string(j);
            m_var_pi_fir[i][j] = m_model.addVar(0.0, GRB_INFINITY, 0.0, GRB_INTEGER, varName);
        }
    }

    // Déclaration des pi_s
    m_var_pi_s.resize(NbStage);
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string varName = "pi_s_" + std::to_string(i);
        m_var_pi_s[i] = m_model.addVar(0.0, PiMax, 0.0, GRB_INTEGER, varName);
    }

    // Déclaration des a
    m_var_a.resize(NbStage);
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string varName = "a_" + std::to_string(i);
        m_var_a[i] = m_model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, varName);
    }

    // Déclaration des r
    m_var_r.resize(NbStage);
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string varName = "r_" + std::to_string(i);
        m_var_r[i] = m_model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, varName);
    }

    // Déclaration des pi
    m_var_pi.resize(NbStage);
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string varName = "pi_" + std::to_string(i);
        m_var_pi[i] = m_model.addVar(0.0, PiMax, 0.0, GRB_INTEGER, varName);
    }

    // Déclaration des PI_IN
    {
        std::string varName = "PI_IN";
        m_var_PI_IN = m_model.addVar(PiIn, PiIn, 0.0, GRB_INTEGER, varName);
    }

    // Déclaration des contraintes
    // Un filtre au plus par étage
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string cstrName = "cstr_nb_fir_" + std::to_string(i);
        GRBLinExpr expr = 0;
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            expr += m_var_delta[i][j];
        }
        m_model.addConstr(expr, GRB_LESS_EQUAL, 1.0, cstrName);
    }

    // Définition de la taille occupée
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string cstrName = "cstr_a_" + std::to_string(i);
        GRBQuadExpr expr = 0;

        // Affectation de la contrainte à a_i
        expr -= m_var_a[i];

        // Contrainte NON LINEAIRE
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            const Fir &currentFir = m_firs[j];

            if (i == 0) {
                expr += m_var_delta[i][j] * currentFir.getCardC() * (currentFir.getPiC() + m_var_PI_IN);
            }
            else {
                expr += m_var_delta[i][j] * currentFir.getCardC() * (currentFir.getPiC() + m_var_pi[i-1]);
            }
        }
        m_model.addQConstr(expr, GRB_EQUAL, 0.0, cstrName);
    }

    // Définition de la rejection
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string cstrName = "cstr_r_" + std::to_string(i);
        GRBLinExpr expr = 0;

        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            const Fir &currentFir = m_firs[j];
            expr += m_var_delta[i][j] * currentFir.getNoiseLevel();
        }

        // Affectation de la contrainte à r_i
        expr += -m_var_r[i];

        m_model.addConstr(expr, GRB_EQUAL, 0.0, cstrName);
    }

    // Contrainte sur la taille en sortie
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string cstrName = "cstr_pi_i_min_" + std::to_string(i);
        GRBLinExpr expr = 0;

        // Somme des rejections précédentes (avec shift)
        for (int stage = 0; stage <= i; ++stage) {
            expr += (1.0/6.0) * m_var_r[stage];

            // Pour prendre en compte le bit de signe
            expr += 1;

            // TROP LENT !
            // // Pour prendre en compte le bit de signe - si un filtre est séléctionné
            // GRBLinExpr sum_delta = 0;
            // for (std::int64_t j = 0; j < NbConfFir; ++j) {
            //   sum_delta += m_var_delta[i][j];
            // }
            // expr += sum_delta;
        }

        // Ajout d'un bit de sécurité (Utile ?)
        expr += 1;

        m_model.addConstr(expr, GRB_LESS_EQUAL, m_var_pi[i], cstrName);
    }

    // Définition de pi_fir
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            const Fir &currentFir = m_firs[j];
            std::string cstrName = "cstr_pi_fir_" + std::to_string(i) + "_" + std::to_string(j);
            m_model.addConstr(m_var_delta[i][j] * currentFir.getPiFir() - m_var_pi_fir[i][j] == 0, cstrName);
        }
    }

    // Définition de la taille des données en sortie à chaque étage
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string cstrName = "cstr_pi_" + std::to_string(i);
        GRBQuadExpr expr = 0;

        // Affectation de la contrainte à pi
        expr -= m_var_pi[i];

        // La taille de l'étage = taille en sortie du filtre moins le shift
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            expr += m_var_pi_fir[i][j] - m_var_delta[i][j] * m_var_pi_s[i];
        }

        // Récupération de la taille d'entrée des données
        if (i == 0) {
            expr += m_var_PI_IN;
        }
        else {
            expr += m_var_pi[i-1];
        }

        m_model.addQConstr(expr, GRB_EQUAL, 0.0, cstrName);
    }

    // Contrainte sur la taille max
    {
        std::string cstrName = "cstr_A_max";
        GRBLinExpr expr = 0;
        for (std::int64_t i = 0; i < NbStage; ++i) {
            expr += m_var_a[i];
        }
        m_model.addConstr(expr, GRB_LESS_EQUAL, AMax, cstrName);
    }

    // Set the objective
    {
        GRBLinExpr expr = 0;
        for (int i = 0; i < NbStage; ++i) {
            expr += m_var_r[i];
        }
        m_model.setObjective(expr, GRB_MAXIMIZE);
    }

    auto tStart = std::chrono::high_resolution_clock::now();

    // Execute le programme linéaire
    m_model.optimize();

    auto tEnd = std::chrono::high_resolution_clock::now();

    m_computationTime = std::chrono::duration<double>(tEnd-tStart).count();
    std::cout << "Wall clock time passed: " << m_computationTime << "s" << std::endl;

    for (int i = 0; i < NbStage; ++i) {
        for (int j = 0; j < NbConfFir; ++j) {
            bool selected = static_cast<bool>(std::round(m_var_delta[i][j].get(GRB_DoubleAttr_X)));

            if (selected) {
                Fir &fir = m_firs[j];
                double rejection = m_var_r[i].get(GRB_DoubleAttr_X);
                std::int64_t shift = std::round(m_var_pi_s[i].get(GRB_DoubleAttr_X));
                std::int64_t piIn = 0;
                if (i == 0) {
                    piIn = std::round(m_var_PI_IN.get(GRB_DoubleAttr_X));
                }
                else {
                    piIn = std::round(m_var_pi[i - 1].get(GRB_DoubleAttr_X));
                }
                std::int64_t piFir = std::round(m_var_pi_fir[i][j].get(GRB_DoubleAttr_X));
                std::int64_t piOut = std::round(m_var_pi[i].get(GRB_DoubleAttr_X));

                SelectedFilter filter = { i, fir, rejection, shift, piIn, piFir, piOut };
                m_selectedFilters.emplace_back(filter);
            }
        }
    }

    // Calcul des valeurs importantes
    for (std::int64_t i = 0; i < NbStage; ++i) {
        m_areaValue += m_var_a[i].get(GRB_DoubleAttr_X);
        m_rejectionValue += m_var_r[i].get(GRB_DoubleAttr_X);
    }
    m_lastPi = m_var_pi[NbStage - 1].get(GRB_DoubleAttr_X);
}

const std::vector<SelectedFilter> &MaximizeRejection::getSelectedFilters() const {
    return m_selectedFilters;
}

void MaximizeRejection::printResults(std::ostream &out) {
    m_model.update();

    out << std::endl;
    out << "Computation Time = " << m_computationTime << " seconds" << std::endl;

    out << std::endl;
    out << "### Main criteria ###" << std::endl;
    out << "Objectif = " << m_model.get(GRB_DoubleAttr_ObjVal) << std::endl;
    out << "Area = " << m_areaValue << std::endl;
    out << "Rejection = " << m_rejectionValue << std::endl;
    out << "Last pi_i = " << m_lastPi << std::endl;

    out << std::endl;
    out << "### Selected filters ###" << std::endl;
    int i = 0;
    for (const SelectedFilter &filter: m_selectedFilters) {
        out << "Stage #" << filter.stage << std::endl;
        out << filter.filter << std::endl;
        out << "pi_in: " << filter.piIn << std::endl;
        out << "pi_fir: " << filter.piFir << std::endl;
        out << "pi_out: " << filter.piOut << std::endl;
        out << "r_i: " << filter.rejection << std::endl;
        out << "r_i/6: " << filter.rejection / 6.0 << std::endl;
        out << "With shift: " << filter.shift << std::endl;
        out << "Stage rejection: " << m_var_r[i].get(GRB_DoubleAttr_X) << std::endl;
        ++i;
    }

    out << std::endl;
    out << "### Command for the C++ simulator" << std::endl;
    out << "./cascaded-filters data_prn.bin simu_" << m_selectedFilters.size() << "_stage.bin ";
    for (std::size_t stage = 0; stage < m_selectedFilters.size(); ++stage) {
        const SelectedFilter &filter = m_selectedFilters[stage];
        out << filter.filter.getFilterName() << " " << filter.shift << " " << filter.piOut << " ";
    }
    out << std::endl;
}
