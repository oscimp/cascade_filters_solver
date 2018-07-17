#include "LinearProgram.h"

#include <cassert>
#include <cmath>
#include <iostream>

LinearProgram::LinearProgram(const std::int64_t nbStage, const double areaMax, const std::string &firlsFile, const std::string &fir1File, const std::string &outputFormat)
: m_mip(nullptr)
, m_areaValue(0.0)
, m_rejectionValue(0.0)
, m_lastPi(0.0)
, m_outputFormat(outputFormat) {
    // Load firls coeffcients
    loadFirConfiguration(firlsFile, FirMethod::FirLS);
    loadFirConfiguration(fir1File, FirMethod::Fir1);

    // Initialisation du mip
    m_mip = glp_create_prob();
    glp_set_prob_name(m_mip, "Filter Chain Dual");

    // Déclaration des constantes internes
    const std::int64_t NbConfFir = m_firs.size();
    const std::int64_t NbStage = nbStage;
    const std::int64_t NbTotalConf = NbConfFir * NbStage;
    const std::int64_t NbRows =  5 * NbStage + 7 * NbTotalConf + 1;
    const std::int64_t NbCols =  4 * NbTotalConf + 4 * NbStage + 1;
    const std::int64_t PiIn = 16;
    const std::int64_t PiMax = 512;
    const double AMax = areaMax;

    // Déclaration du nombre de colonnes
    glp_add_rows(m_mip, NbRows);

    // Helper pour les contraintes
    // cstr_a: 1, NbStage
    auto cstr_a = [NbStage](int i) {
        assert(0 <= i && i < NbStage);
        return 1 + i;
    };

    // cstr_r: NbStage + 1, NbStage * 2
    auto cstr_r = [NbStage](int i) {
        assert(0 <= i && i < NbStage);
        return NbStage + 1 + i;
    };

    // cstr_pi: 2 * NbStage + 1, NbStage * 3
    auto cstr_pi = [NbStage](int i) {
        assert(0 <= i && i < NbStage);
        return 2 * NbStage + 1 + i;
    };

    // cstr_zeta_eq1: 3 * NbStage + 1, NbStage * 3 + NbTotalConf
    auto cstr_zeta_eq1 = [NbStage, NbConfFir](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 3 * NbStage + 1 + i * NbConfFir + j;
    };

    // cstr_zeta_eq2: 3 * NbStage + NbTotalConf + 1, NbStage * 3 + 2 * NbTotalConf
    auto cstr_zeta_eq2 = [NbStage, NbConfFir, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 3 * NbStage + NbTotalConf + 1 + i * NbConfFir + j;
    };

    // cstr_zeta_eq3: 3 * NbStage + 2 * NbTotalConf + 1, NbStage * 3 + 3 * NbTotalConf
    auto cstr_zeta_eq3 = [NbStage, NbConfFir, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 3 * NbStage + 2 * NbTotalConf + 1 + i * NbConfFir + j;
    };

    // cstr_psi_eq1: 3 * NbStage + 3 * NbTotalConf + 1, NbStage * 3 + 4 * NbTotalConf
    auto cstr_psi_eq1 = [NbStage, NbConfFir, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 3 * NbStage + 3 * NbTotalConf + 1 + i * NbConfFir + j;
    };

    // cstr_psi_eq2: 3 * NbStage + 4 * NbTotalConf + 1, NbStage * 3 + 5 * NbTotalConf
    auto cstr_psi_eq2 = [NbStage, NbConfFir, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 3 * NbStage + 4 * NbTotalConf + 1 + i * NbConfFir + j;
    };

    // cstr_psi_eq3: 3 * NbStage + 5 * NbTotalConf + 1, NbStage * 3 + 6 * NbTotalConf
    auto cstr_psi_eq3 = [NbStage, NbConfFir, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 3 * NbStage + 5 * NbTotalConf + 1 + i * NbConfFir + j;
    };

    // cstr_nb_fir: 3 * NbStage + 6 * NbTotalConf + 1, NbStage * 4 + 6 * NbTotalConf
    auto cstr_nb_fir = [NbStage, NbConfFir, NbTotalConf](int i) {
        assert(0 <= i && i < NbStage);
        return 3 * NbStage + 6 * NbTotalConf + 1 + i;
    };

    // cstr_pi_s: 4 * NbStage + 6 * NbTotalConf + 1, NbStage * 5 + 6 * NbTotalConf
    auto cstr_pi_s = [NbStage, NbConfFir, NbTotalConf](int i) {
        assert(0 <= i && i < NbStage);
        return 4 * NbStage + 6 * NbTotalConf + 1 + i;
    };

    // cstr_pi_fir: 5 * NbStage + 6 * NbTotalConf + 1, 5 * NbStage + 7 * NbTotalConf
    auto cstr_pi_fir = [NbStage, NbConfFir, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 5 * NbStage + 6 * NbTotalConf + 1 + i * NbConfFir + j;
    };

    // cstr_A: 5 * NbStage + 7 * NbTotalConf + 1
    const int cstr_A = NbStage * 5 + 7 * NbTotalConf + 1;

    // Define the rows
    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameRow = "cstr_a_" + std::to_string(i);
        glp_set_row_name(m_mip, cstr_a(i), nameRow.c_str());
        glp_set_row_bnds(m_mip, cstr_a(i), GLP_FX, 0.0, 0.0); // = 0
#ifdef DEBUG
        std::cout << nameRow << ": " << cstr_a(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameRow = "cstr_r_" + std::to_string(i);
        glp_set_row_name(m_mip, cstr_r(i), nameRow.c_str());
        glp_set_row_bnds(m_mip, cstr_r(i), GLP_FX, 0.0, 0.0); // = 0
#ifdef DEBUG
        std::cout << nameRow << ": " << cstr_r(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameRow = "cstr_pi_" + std::to_string(i);
        glp_set_row_name(m_mip, cstr_pi(i), nameRow.c_str());
        glp_set_row_bnds(m_mip, cstr_pi(i), GLP_FX, 0.0, 0.0); // = 0
#ifdef DEBUG
        std::cout << nameRow << ": " << cstr_pi(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameRow = "cstr_zeta_eq1_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_row_name(m_mip, cstr_zeta_eq1(i, j), nameRow.c_str());
            glp_set_row_bnds(m_mip, cstr_zeta_eq1(i, j), GLP_LO, 0.0, 0.0); // [0.0 ; +inf]
#ifdef DEBUG
            std::cout << nameRow << ": " << cstr_zeta_eq1(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameRow = "cstr_zeta_eq2_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_row_name(m_mip, cstr_zeta_eq2(i, j), nameRow.c_str());
            glp_set_row_bnds(m_mip, cstr_zeta_eq2(i, j), GLP_LO, 0.0, 0.0); // [0.0 ; +inf]
#ifdef DEBUG
            std::cout << nameRow << ": " << cstr_zeta_eq2(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameRow = "cstr_zeta_eq3_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_row_name(m_mip, cstr_zeta_eq3(i, j), nameRow.c_str());
            glp_set_row_bnds(m_mip, cstr_zeta_eq3(i, j), GLP_UP, 0.0, PiMax); // [-inf ; PiMax]
#ifdef DEBUG
            std::cout << nameRow << ": " << cstr_zeta_eq3(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameRow = "cstr_psi_eq1_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_row_name(m_mip, cstr_psi_eq1(i, j), nameRow.c_str());
            glp_set_row_bnds(m_mip, cstr_psi_eq1(i, j), GLP_LO, 0.0, 0.0); // [0.0 ; +inf]
#ifdef DEBUG
            std::cout << nameRow << ": " << cstr_psi_eq1(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameRow = "cstr_psi_eq2_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_row_name(m_mip, cstr_psi_eq2(i, j), nameRow.c_str());
            glp_set_row_bnds(m_mip, cstr_psi_eq2(i, j), GLP_LO, 0.0, 0.0); // [0.0 ; +inf]
#ifdef DEBUG
            std::cout << nameRow << ": " << cstr_psi_eq2(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameRow = "cstr_psi_eq3_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_row_name(m_mip, cstr_psi_eq3(i, j), nameRow.c_str());
            glp_set_row_bnds(m_mip, cstr_psi_eq3(i, j), GLP_UP, 0.0, PiMax); // [-inf ; PiMax]
#ifdef DEBUG
            std::cout << nameRow << ": " << cstr_psi_eq3(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameRow = "cstr_nb_fir_" + std::to_string(i);
        glp_set_row_name(m_mip, cstr_nb_fir(i), nameRow.c_str());
        glp_set_row_bnds(m_mip, cstr_nb_fir(i), GLP_UP, 0.0, 1.0); // [0 ; 1]
        // glp_set_row_bnds(m_mip, cstr_nb_fir(i), GLP_FX, 1.0, 1.0); // = 1
#ifdef DEBUG
        std::cout << nameRow << ": " << cstr_nb_fir(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameRow = "cstr_pi_s_" + std::to_string(i);
        glp_set_row_name(m_mip, cstr_pi_s(i), nameRow.c_str());
        glp_set_row_bnds(m_mip, cstr_pi_s(i), GLP_LO, 0.0, 0.0); // [0 ; +inf]
#ifdef DEBUG
        std::cout << nameRow << ": " << cstr_pi_s(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameRow = "cstr_pi_fir_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_row_name(m_mip, cstr_pi_fir(i, j), nameRow.c_str());
            glp_set_row_bnds(m_mip, cstr_pi_fir(i, j), GLP_FX, 0.0, 0.0); // = 0
#ifdef DEBUG
            std::cout << nameRow << ": " << cstr_pi_fir(i, j) << std::endl;
#endif // DEBUG
        }
    }

    {
        std::string nameRow = "cstr_A_TOTAL";
        glp_set_row_name(m_mip, cstr_A, nameRow.c_str());
        glp_set_row_bnds(m_mip, cstr_A, GLP_UP, 0.0, AMax); // [ -inf ; AMax ]
#ifdef DEBUG
        std::cout << nameRow << ": " << cstr_A << std::endl;
#endif // DEBUG
    }

#ifdef DEBUG
    std::cout << "Total Rows: " << std::to_string(NbRows) << std::endl;
#endif // DEBUG

    // Déclaration du nombre de lignes
    glp_add_cols(m_mip, NbCols);

    // Helper pour les variables
    // delta: 1, NbTotalConf
    auto var_delta = [NbConfFir, NbStage](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 1 + i * NbConfFir + j;
    };

    // zeta: NbTotalConf + 1, 2 * NbTotalConf + 1
    auto var_zeta = [NbConfFir, NbStage, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return NbTotalConf + 1 + i * NbConfFir + j;
    };

    // psi: 2 * NbTotalConf + 1, 3 * NbTotalConf
    auto var_psi = [NbConfFir, NbStage, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 2 * NbTotalConf + 1 + i * NbConfFir + j;
    };

    // pi_s: 3 * NbTotalConf + 1, 3 * NbTotalConf + NbStage
    auto var_pi_s = [NbConfFir, NbStage, NbTotalConf](int i) {
        assert(0 <= i && i < NbStage);
        return 3 * NbTotalConf + 1 + i;
    };

    // a: 3 * NbTotalConf + NbStage + 1, 3 * NbTotalConf + 2 * NbStage
    auto var_a = [NbConfFir, NbStage, NbTotalConf](int i) {
        assert(0 <= i && i < NbStage);
        return 3 * NbTotalConf + NbStage + 1 + i;
    };

    // r: 3 * NbTotalConf + 2 * NbStage + 1, 3 * NbTotalConf + 3 * NbStage
    auto var_r = [NbConfFir, NbStage, NbTotalConf](int i) {
        assert(0 <= i && i < NbStage);
        return 3 * NbTotalConf + 2 * NbStage + 1 + i;
    };

    // pi: 3 * NbTotalConf + 3 * NbStage + 1, 3 * NbTotalConf + 4 * NbStage
    auto var_pi = [NbConfFir, NbStage, NbTotalConf](int i) {
        assert(0 <= i && i < NbStage);
        return 3 * NbTotalConf + 3 * NbStage + 1 + i;
    };

    // psi: 3 * NbTotalConf + 4 * NbStage + 1, 4 * NbTotalConf + 4 * NbStage
    auto var_pi_fir = [NbConfFir, NbStage, NbTotalConf](int i, int j) {
        assert(0 <= i && i < NbStage);
        assert(0 <= j && j < NbConfFir);
        return 3 * NbTotalConf + 4 * NbStage + 1 + i * NbConfFir + j;
    };

    const int var_PI_IN = 4 * NbTotalConf + 4 * NbStage + 1;

#ifdef DEBUG
    for (Fir &fir: m_firs) {
        std::cout << fir.getPiC() << " " << fir.getCardC() << " " << fir.getNoiseLevel() << std::endl;
    }
#endif // DEBUG

    // Declare the cols
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameCol = "delta_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_col_name(m_mip, var_delta(i, j), nameCol.c_str());
            glp_set_col_kind(m_mip, var_delta(i, j), GLP_BV);
#ifdef DEBUG
            std::cout << nameCol << ": " << var_delta(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameCol = "zeta_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_col_name(m_mip, var_zeta(i, j), nameCol.c_str());
            glp_set_col_kind(m_mip, var_zeta(i, j), GLP_CV);
            glp_set_col_bnds(m_mip, var_zeta(i, j), GLP_LO, 0.0, 0.0); // [ 0; +inf ]
#ifdef DEBUG
            std::cout << nameCol << ": " << var_zeta(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameCol = "psi_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_col_name(m_mip, var_psi(i, j), nameCol.c_str());
            glp_set_col_kind(m_mip, var_psi(i, j), GLP_CV);
            glp_set_col_bnds(m_mip, var_psi(i, j), GLP_LO, 0.0, 0.0); // [ 0; +inf ]
#ifdef DEBUG
            std::cout << nameCol << ": " << var_psi(i, j) << std::endl;
#endif // DEBUG
        }
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameCol = "pi_s_" + std::to_string(i);
        glp_set_col_name(m_mip, var_pi_s(i), nameCol.c_str());
        glp_set_col_kind(m_mip, var_pi_s(i), GLP_IV);
        glp_set_col_bnds(m_mip, var_pi_s(i), GLP_LO, 0.0, 0.0); // [ 0; +inf ]
#ifdef DEBUG
        std::cout << nameCol << ": " << var_pi_s(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameCol = "a_" + std::to_string(i);
        glp_set_col_name(m_mip, var_a(i), nameCol.c_str());
        glp_set_col_kind(m_mip, var_a(i), GLP_CV);
        glp_set_col_bnds(m_mip, var_a(i), GLP_FR, 0.0, 0.0); // [-inf; +inf]
#ifdef DEBUG
        std::cout << nameCol << ": " << var_a(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameCol = "r_" + std::to_string(i);
        glp_set_col_name(m_mip, var_r(i), nameCol.c_str());
        glp_set_col_kind(m_mip, var_r(i), GLP_CV);
        glp_set_col_bnds(m_mip, var_r(i), GLP_FR, 0.0, 0.0); // [-inf; +inf]
#ifdef DEBUG
        std::cout << nameCol << ": " << var_r(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        std::string nameCol = "pi_" + std::to_string(i);
        glp_set_col_name(m_mip, var_pi(i), nameCol.c_str());
        glp_set_col_kind(m_mip, var_pi(i), GLP_CV);
        glp_set_col_bnds(m_mip, var_pi(i), GLP_LO, 0.0, 0.0); // [ 0; +inf ]
#ifdef DEBUG
        std::cout << nameCol << ": " << var_pi(i) << std::endl;
#endif // DEBUG
    }

    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            std::string nameCol = "pi_fir_" + std::to_string(i) + "_" + std::to_string(j);
            glp_set_col_name(m_mip, var_pi_fir(i, j), nameCol.c_str());
            glp_set_col_kind(m_mip, var_pi_fir(i, j), GLP_CV);
            glp_set_col_bnds(m_mip, var_pi_fir(i, j), GLP_LO, 0.0, 0.0); // [ 0; +inf ]
#ifdef DEBUG
            std::cout << nameCol << ": " << var_pi_fir(i, j) << std::endl;
#endif // DEBUG
        }
    }

    {
        std::string nameCol = "PI_IN";
        glp_set_col_name(m_mip, var_PI_IN, nameCol.c_str());
        glp_set_col_kind(m_mip, var_PI_IN, GLP_CV);
        glp_set_col_bnds(m_mip, var_PI_IN, GLP_FX, PiIn, PiIn); // = PiIn
#ifdef DEBUG
        std::cout << nameCol << ": " << var_PI_IN << std::endl;
#endif // DEBUG
    }

#ifdef DEBUG
    std::cout << "Total Cols: " << std::to_string(NbCols) << std::endl;
#endif // DEBUG

    // Définition de l'objectif
    glp_set_obj_dir(m_mip, GLP_MAX);

    // Pour tous les étages de rejection
    for (std::int64_t i = 0; i < NbStage; ++i) {
        glp_set_obj_coef(m_mip, var_r(i), 1);
    }

    // Première ligne vide
    m_constraints.push_back(0);
    m_variables.push_back(0);
    m_coefficients.push_back(0);

    // Définition d'un fir par étage
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            m_constraints.push_back(cstr_nb_fir(i));
            m_variables.push_back(var_delta(i, j));
            m_coefficients.push_back(1);
        }
    }

    // Définition de la place occupée
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            Fir &currentFir = m_firs[j];
            m_constraints.push_back(cstr_a(i));
            m_variables.push_back(var_delta(i, j));
            m_coefficients.push_back(currentFir.getCardC() * currentFir.getPiC());

            m_constraints.push_back(cstr_a(i));
            m_variables.push_back(var_zeta(i, j));
            m_coefficients.push_back(currentFir.getCardC());
        }

        // Affectation de la valeur de la contrainte à a_i
        m_constraints.push_back(cstr_a(i));
        m_variables.push_back(var_a(i));
        m_coefficients.push_back(-1);
    }

    // Définition de la réjéction
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            Fir &currentFir = m_firs[j];
            m_constraints.push_back(cstr_r(i));
            m_variables.push_back(var_delta(i, j));
            m_coefficients.push_back(currentFir.getNoiseLevel());
        }

        // Affectation de la valeur de la contrainte à r_i
        m_constraints.push_back(cstr_r(i));
        m_variables.push_back(var_r(i));
        m_coefficients.push_back(-1);
    }

    // Définition de la contrainte de pi_s
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            m_constraints.push_back(cstr_pi_s(i));
            m_variables.push_back(var_pi_fir(i, j));
            m_coefficients.push_back(1);
        }

        m_constraints.push_back(cstr_pi_s(i));
        m_variables.push_back(var_r(i));
        m_coefficients.push_back(-1.0/6.0);

        m_constraints.push_back(cstr_pi_s(i));
        m_variables.push_back(var_pi_s(i));
        m_coefficients.push_back(-1);
    }
    // Taille des données dépendent de la taille précédente
    for (std::int64_t i = 1; i < NbStage; ++i) {
        m_constraints.push_back(cstr_pi_s(i));
        m_variables.push_back(var_pi(i - 1));
        m_coefficients.push_back(1);
    }
    // Cas particulier pour pi_0
    m_constraints.push_back(cstr_pi_s(0));
    m_variables.push_back(var_PI_IN);
    m_coefficients.push_back(1);

    // Définition de pi_fir
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            const Fir &currentFir = m_firs[j];
            m_constraints.push_back(cstr_pi_fir(i, j));
            m_variables.push_back(var_delta(i, j));
            m_coefficients.push_back(currentFir.getPiC() + std::ceil(std::log2(currentFir.getCardC())));

            // Affectation de pi_fir
            m_constraints.push_back(cstr_pi_fir(i, j));
            m_variables.push_back(var_pi_fir(i, j));
            m_coefficients.push_back(-1);
        }
    }

    // Définition de la taille des données
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            m_constraints.push_back(cstr_pi(i));
            m_variables.push_back(var_pi_fir(i, j));
            m_coefficients.push_back(1); // currentFir.getPiC() + std::ceil(std::log2(currentFir.getCardC()))

            m_constraints.push_back(cstr_pi(i));
            m_variables.push_back(var_psi(i, j));
            m_coefficients.push_back(-1);
        }

        // Affectation de la valeur de la contrainte à pi_i
        m_constraints.push_back(cstr_pi(i));
        m_variables.push_back(var_pi(i));
        m_coefficients.push_back(-1);
    }

    // Taille des données dépendent de la taille précédente
    for (std::int64_t i = 1; i < NbStage; ++i) {
        m_constraints.push_back(cstr_pi(i));
        m_variables.push_back(var_pi(i - 1));
        m_coefficients.push_back(1);
    }
    // Cas particulier pour pi_0
    m_constraints.push_back(cstr_pi(0));
    m_variables.push_back(var_PI_IN);
    m_coefficients.push_back(1);

    // Définition des contraintes pour zeta
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            // cstr_zeta_eq1
            m_constraints.push_back(cstr_zeta_eq1(i, j));
            m_variables.push_back(var_zeta(i, j));
            m_coefficients.push_back(-1);
            m_constraints.push_back(cstr_zeta_eq1(i, j));
            m_variables.push_back(var_delta(i, j));
            m_coefficients.push_back(PiMax);

            // cstr_zeta_eq2
            m_constraints.push_back(cstr_zeta_eq2(i, j));
            m_variables.push_back(var_zeta(i, j));
            m_coefficients.push_back(-1);
            // Cas de pi_i-1
            if (i == 0) {
                m_constraints.push_back(cstr_zeta_eq2(i, j));
                m_variables.push_back(var_PI_IN);
                m_coefficients.push_back(1);
            }
            else {
                m_constraints.push_back(cstr_zeta_eq2(i, j));
                m_variables.push_back(var_pi(i - 1));
                m_coefficients.push_back(1);
            }

            // cstr_zeta_eq3
            m_constraints.push_back(cstr_zeta_eq3(i, j));
            m_variables.push_back(var_zeta(i, j));
            m_coefficients.push_back(-1);
            m_constraints.push_back(cstr_zeta_eq3(i, j));
            m_variables.push_back(var_delta(i, j));
            m_coefficients.push_back(PiMax);
            // Cas de pi_i-1
            if (i == 0) {
                m_constraints.push_back(cstr_zeta_eq3(i, j));
                m_variables.push_back(var_PI_IN);
                m_coefficients.push_back(1);
            }
            else {
                m_constraints.push_back(cstr_zeta_eq3(i, j));
                m_variables.push_back(var_pi(i - 1));
                m_coefficients.push_back(1);
            }
        }
    }

    // Définition des contraintes pour psi
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            // cstr_psi_eq1
            m_constraints.push_back(cstr_psi_eq1(i, j));
            m_variables.push_back(var_psi(i, j));
            m_coefficients.push_back(-1);
            m_constraints.push_back(cstr_psi_eq1(i, j));
            m_variables.push_back(var_delta(i, j));
            m_coefficients.push_back(PiMax);

            // cstr_psi_eq2
            m_constraints.push_back(cstr_psi_eq2(i, j));
            m_variables.push_back(var_psi(i, j));
            m_coefficients.push_back(-1);
            m_constraints.push_back(cstr_psi_eq2(i, j));
            m_variables.push_back(var_pi_s(i));
            m_coefficients.push_back(1);

            // cstr_psi_eq3
            m_constraints.push_back(cstr_psi_eq3(i, j));
            m_variables.push_back(var_psi(i, j));
            m_coefficients.push_back(-1);
            m_constraints.push_back(cstr_psi_eq3(i, j));
            m_variables.push_back(var_delta(i, j));
            m_coefficients.push_back(PiMax);
            m_constraints.push_back(cstr_psi_eq3(i, j));
            m_variables.push_back(var_pi_s(i));
            m_coefficients.push_back(1);
        }
    }

    // Taille = somme des a_i
    for (std::int64_t i = 0; i < NbStage; ++i) {
        m_constraints.push_back(cstr_A);
        m_variables.push_back(var_a(i));
        m_coefficients.push_back(1);
    }

    // Execute le programme linéaire
    glp_load_matrix(m_mip, m_coefficients.size() - 1, m_constraints.data(), m_variables.data(), m_coefficients.data());
    glp_simplex(m_mip, nullptr);
    glp_intopt(m_mip, nullptr);

    // Sauvegarde le choix des filtres
    for (std::int64_t i = 0; i < NbStage; ++i) {
        for (std::int64_t j = 0; j < NbConfFir; ++j) {
            if (glp_mip_col_val(m_mip, var_delta(i, j)) == 1) {
                double rejection = glp_mip_col_val(m_mip, var_r(i));
                std::int64_t piIn = 0;
                if (i == 0) {
                    piIn = glp_mip_col_val(m_mip, var_PI_IN);
                }
                else {
                    piIn = glp_mip_col_val(m_mip, var_pi(i - 1));
                }
                std::int64_t piFir = glp_mip_col_val(m_mip, var_pi_fir(i, j));
                std::int64_t piOut = glp_mip_col_val(m_mip, var_pi(i));
                std::int64_t shift = glp_mip_col_val(m_mip, var_pi_s(i));
                SelectedFilter filter = { i + 1, m_firs[j], rejection, shift, piIn, piFir, piOut };
                m_selectedFilters.emplace_back(filter);
                break;
            }
        }
    }

    // Calcul des valeurs importantes
    for (std::int64_t i = 0; i < NbStage; ++i) {
        m_areaValue += glp_mip_col_val(m_mip, var_a(i));
        m_rejectionValue += glp_mip_col_val(m_mip, var_r(i));
    }
    m_lastPi = glp_mip_col_val(m_mip, var_pi(NbStage - 1));
}

LinearProgram::~LinearProgram() {
    if (m_mip != nullptr) {
        glp_delete_prob(m_mip);
    }
}

const std::vector<SelectedFilter> &LinearProgram::getSelectedFilters() const {
    return m_selectedFilters;
}

void LinearProgram::printDebugFile() {
    std::cout << std::endl;
    std::cout << "### Write the linear programm and the solution ###" << std::endl;

    std::string filename = m_outputFormat + "/glpk_lp.txt";
    glp_write_lp(m_mip, NULL, filename.c_str());

    filename = m_outputFormat + "/glpk_lp_sol.txt";
    glp_print_sol(m_mip, filename.c_str());

    filename = m_outputFormat + "/glpk_mip_sol.txt";
    glp_print_mip(m_mip, filename.c_str());
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
    out << std::endl;
    out << "### Main criteria ###" << std::endl;
    out << "Objectif = " << glp_mip_obj_val(m_mip) << std::endl;
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
