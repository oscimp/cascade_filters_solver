#ifndef TCL_PROJECT_H
#define TCL_PROJECT_H

#include <fstream>

class Fir;
class LinearProgram;
class SelectedFilter;

class TclProject {
public:
    void generate(const LinearProgram &lp, const std::string &outputFormat);

protected:
    void generateProjectFile(const LinearProgram &lp, const std::string &outputFormat);
    virtual void writeTclHeader(std::ofstream &file, const std::string &outputFormat) = 0;
    virtual void addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, std::string &previousSource);
    virtual void writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &outputFormat) = 0;
};

#endif // TCL_PROJECT_H
