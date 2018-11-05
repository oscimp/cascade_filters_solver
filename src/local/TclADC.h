#ifndef TCL_ADC_H
#define TCL_ADC_H

#include <fstream>

#include "TclProject.h"

class TclADC: public TclProject {
private:
    void writeTclHeader(std::ofstream &file, const std::string &outputFormat);
    virtual void addTclFir(std::ofstream &file, int firNumber, const SelectedFilter &filter, std::string &previousSource) override;
    void writeTclFooter(std::ofstream &file, int inputSize, std::string &previousSource, const std::string &outputFormat);
};

#endif // TCL_ADC_H
