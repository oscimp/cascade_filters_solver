#ifndef SCRIPT_GENERATOR
#define SCRIPT_GENERATOR

#include <iostream>

class LinearProgram;

class ScriptGenerator {
public:
    static void generateDeployScript(const LinearProgram &lp, const std::string &outputFormat, const std::string dtboType);
    static void generateSimulationScript(const LinearProgram &lp, const std::string &outputFormat);

private:
    static std::ofstream createShellFile(const std::string &scriptFilename);
    static void safeShellCommand(std::ofstream &file, const std::string &command, int expectedReturn = 0);
    static std::ofstream createOctaveFile(const std::string &scriptFilename);
};

#endif // SCRIPT_GENERATOR
