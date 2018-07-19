#ifndef LINEAR_PROGRAM_H
#define LINEAR_PROGRAM_H

#include <ostream>
#include <string>
#include <vector>

#include <gurobi_c++.h>

#include "Fir.h"

struct SelectedFilter {
    std::int64_t stage;
    Fir filter;
    double rejection;
    std::int64_t shift;
    std::int64_t piIn;
    std::int64_t piFir;
    std::int64_t piOut;
};

class LinearProgram {
public:
    LinearProgram(const std::int64_t nbStage, const double areaMax, const std::string &firlsFile, const std::string &fir1File, const std::string &outputFormat);

    const std::vector<SelectedFilter> &getSelectedFilters() const;

    void printDebugFile();
    void printResults();
    void printResults(const std::string &filename);

private:
    /*! \brief Load FIR confiuration form binary file
     * The format of binary file is :
     * 1) uint16 (number of coefficients)
     * 2) uint16 (number of bit for each coefficient)
     * 2) double (noise rejection)
     *
     * \param filename Binary filename
     * \param method Algorithm used to create the coefficients
     */
    void loadFirConfiguration(const std::string &filename, FirMethod method);

    void printResults(std::ostream &out);

private:
    std::vector<Fir> m_firs;

    GRBEnv m_env;
    GRBModel m_model;
    std::vector< std::vector<GRBVar> > m_var_delta;
    std::vector< std::vector<GRBVar> > m_var_pi_fir;
    std::vector<GRBVar> m_var_pi_s;
    std::vector<GRBVar> m_var_a;
    std::vector<GRBVar> m_var_r;
    std::vector<GRBVar> m_var_pi;
    GRBVar m_var_PI_IN;

    std::vector<SelectedFilter> m_selectedFilters;
    double m_areaValue;
    double m_rejectionValue;
    double m_lastPi;

    std::string m_outputFormat;
};

#endif // LINEAR_PROGRAM_H
