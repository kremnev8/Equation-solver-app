#ifndef EQUATIONPARSER_H
#define EQUATIONPARSER_H

#include <stdexcept>

#include <QString>

#include "equation.h"
#include "utils.h"

class EquationParser {
public:
    static std::string findBrace(std::string& input, size_t start, int& end, char open, char close);
    static void prepareForDisplay(QString& input);
    // Main recirsive function for parsing function input
    static math::Entry* parseEquation(std::string input, int depth);
};

#endif  // EQUATIONPARSER_H
