#ifndef TCL_PROJECT_H
#define TCL_PROJECT_H

#include <fstream>

class Fir;
class LinearProgram;
class SelectedFilter;

class TclProject {
public:
    void generate(const LinearProgram &lp, const std::string &outputFormat);

private:
    void generateProjectFile(const LinearProgram &lp, const std::string &outputFormat);
    void writeTclHeader(std::ofstream &file, const std::string &outputFormat);
    int addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, int inputSize, std::string &previousSource);
    void writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &outputFormat);

private:
    std::string m_fpgaDevPath;
};

#endif // TCL_PROJECT_H
