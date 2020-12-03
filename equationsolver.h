#ifndef EQUATIONSOLVER_H
#define EQUATIONSOLVER_H

#include "equation.h"

class EquationSolver {
public:
    EquationSolver();

    static math::Interval* splitInterval(math::Entry* function, math::Interval* entredInterval, double step, int& iterations);
    // All supported functions of solving for a root
    static bool solveUsingSimpleItterations(math::Entry* equation, math::Entry* cFunc, double a, double b, int precision, double& root, int& iterationCount);
    static bool solveUsingFastItterations(math::Entry* equation, math::Entry* cFunc, double a, double b, int precision, double& root, int& iterationCount);
    static bool solveUsingNewtonMethod(math::Entry* equation, math::Entry* derivative, double a, double b, int precision, double& root, int& iterationCount);
    static bool solveUsingDichotomy(math::Entry* equation, double a, double b, int precision, double& root, int& iterationCount);
};

#endif  // EQUATIONSOLVER_H
