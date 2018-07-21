#include "LinearProgram.h"

#include <cassert>
#include <cmath>
#include <iostream>

LinearProgram::LinearProgram(const std::int64_t nbStage, const double areaMax, const std::string &firlsFile, const std::string &fir1File, const std::string &outputFormat)
: m_env(GRBEnv())
, m_model(m_env)
, m_areaValue(0.0)
, m_rejectionValue(0.0)
, m_lastPi(0.0)
, m_outputFormat(outputFormat) {
    // Load firls coeffcients
    loadFirConfiguration(firlsFile, FirMethod::FirLS);
    loadFirConfiguration(fir1File, FirMethod::Fir1);

    // Déclaration des constantes internes
    const std::int64_t NbConfFir = m_firs.size();
    const std::int64_t NbStage = nbStage;
    const std::int64_t PiIn = 16;
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

    // Contraintes sur pi_s
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string cstrName = "cstr_pi_s_" + std::to_string(i);
        GRBLinExpr expr = 0;

        // Affectation de la contrainte à pi_s
        expr -= m_var_pi_s[i];

        // Taille des données à la sortie du filtre
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            expr += m_var_pi_fir[i][j];
        }

        // Minumum de bit pour voir la rejection
        expr -= (1.0/6.0) * m_var_r[i];

        // Récupération de la taille d'entrée des données
        if (i == 0) {
            expr += m_var_PI_IN;
        }
        else {
            expr += m_var_pi[i-1];
        }

        m_model.addConstr(expr, GRB_GREATER_EQUAL, 0.0, cstrName);
    }

    // Définition de pi_fir
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            const Fir &currentFir = m_firs[j];
            std::string cstrName = "cstr_pi_fir_" + std::to_string(i) + "_" + std::to_string(j);
            m_model.addConstr(m_var_delta[i][j] * (currentFir.getPiC() + std::ceil(std::log2(currentFir.getCardC()))) - m_var_pi_fir[i][j] == 0, cstrName);
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

    // Execute le programme linéaire
    m_model.optimize();

    for (int i = 0; i < NbStage; ++i) {
        for (int j = 0; j < NbConfFir; ++j) {
            bool selected = static_cast<bool>(std::round(m_var_delta[i][j].get(GRB_DoubleAttr_X)));

            if (selected) {
                Fir &fir = m_firs[j];
                double rejection = m_var_r[i].get(GRB_DoubleAttr_X);
                std::int64_t shift = m_var_pi_s[i].get(GRB_DoubleAttr_X);
                std::int64_t piIn = 0;
                if (i == 0) {
                    piIn = m_var_PI_IN.get(GRB_DoubleAttr_X);
                }
                else {
                    piIn = m_var_pi[i - 1].get(GRB_DoubleAttr_X);
                }
                std::int64_t piFir = m_var_pi_fir[i][j].get(GRB_DoubleAttr_X);
                std::int64_t piOut = m_var_pi[i].get(GRB_DoubleAttr_X);

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

const std::vector<SelectedFilter> &LinearProgram::getSelectedFilters() const {
    return m_selectedFilters;
}

void LinearProgram::printDebugFile() {
    std::cout << std::endl;
    std::cout << "### Write the linear programm and the solution ###" << std::endl;
    m_model.update();

    std::string filename = m_outputFormat + "/gurobi.lp";
    m_model.write(filename);

    // filename = m_outputFormat + "/glpk_lp_sol.txt";
    // glp_print_sol(m_mip, filename.c_str());
    //
    // filename = m_outputFormat + "/glpk_mip_sol.txt";
    // glp_print_mip(m_mip, filename.c_str());
}

void LinearProgram::printResults() {
    printResults(std::cout);
}

void LinearProgram::printResults(const std::string &filename) {
    std::ofstream file(m_outputFormat + "/" + filename);
    if(!file.good()) {
        std::cerr << "LinearProgram::printResults(filename): open '" << m_outputFormat << "/" << filename << "': failed" << std::endl;
    }

    printResults(file);
}

void LinearProgram::loadFirConfiguration(const std::string &filename, FirMethod method) {
    // Open the file
    std::ifstream file(filename, std::ios::binary);
    if (file.fail()) {
        std::cerr << "LinearProgram::loadFirConfiguration: The file '" << filename << "' is missing" << std::endl;
        std::exit(-1);
    }

    // Read the file until eof
    for (;;) {
        std::uint16_t nob = 0;
        std::uint16_t coeff = 0;
        double rejection = 0.0;

        // Read the values
        file.read(reinterpret_cast<char*>(&nob), sizeof(std::uint16_t));
        file.read(reinterpret_cast<char*>(&coeff), sizeof(std::uint16_t));
        file.read(reinterpret_cast<char*>(&rejection), sizeof(double));

        // If the end of file is reached
        if (file.eof()) {
            break;
        }

        // Add fir configuration
        m_firs.emplace_back(method, coeff, nob, rejection);
    }
}

void LinearProgram::printResults(std::ostream &out) {
    m_model.update();

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
        ++i;
    }
}
