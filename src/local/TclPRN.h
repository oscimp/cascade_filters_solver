#ifndef TCL_PRN_H
#define TCL_PRN_H

#include <fstream>

#include "TclProject.h"

class TclPRN: public TclProject {
private:
    void writeTclHeader(std::ofstream &file, const std::string &outputFormat);
    void writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &outputFormat);
};

#endif // TCL_PRN_H
