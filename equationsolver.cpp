#include "equationsolver.h"
#include "equationparser.h"

EquationSolver::EquationSolver() {
}

math::Interval* EquationSolver::splitInterval(math::Entry* function, math::Interval* entredInterval, double step, int& iterations) {
    math::Interval* result = new math::Interval();
    iterations = 0;

    for (int i = 0; i < entredInterval->size(); i++) {
        math::Tuple entry = entredInterval->getAt(i);
        for (double x = entry.a; x < entry.b; x += step) {
            double fa = function->evaluate(x)->getValue();
            double fb = function->evaluate(x + step)->getValue();
            if (sign(fa) * sign(fb) == -1) {
                result->addEntry(math::Tuple(x, x + step));
            }
            iterations++;
        }
    }

    return result;
}

bool EquationSolver::solveUsingSimpleItterations(math::Entry* equation, math::Entry* cFunc, double a, double b, int precision, double& root, int& iterationCount) {
    math::Operator* mul = new math::MultiplyFunction();
    mul->addInput(cFunc);
    mul->addInput(equation);

    math::Operator* auxFunc = new math::AddFunction();
    auxFunc->addInput(new math::VariableEntry());
    auxFunc->addInput(mul);

    double xk = a;
    double xk1 = -1;
    int iterations = 0;
    while (true) {
        if (iterations > 100000) {
            return false;
        }
        xk1 = auxFunc->evaluate(xk)->getValue();
        if (abs(xk1 - xk) < pow(10, -precision)) {
            root = xk1;
            iterationCount = iterations;
            return true;
        }
        xk = xk1;
        iterations++;
    }

    return false;
}

bool EquationSolver::solveUsingFastItterations(math::Entry* equation, math::Entry* cFunc, double a, double b, int precision, double& root, int& iterationCount) {
    double xk = a;
    double xk1 = -1;
    int iterations = 0;
    while (true) {
        if (iterations > 100000) {
            return false;
        }
        double cx = cFunc->evaluate(xk)->getValue();
        double fx = equation->evaluate(xk)->getValue();
        xk1 = xk - cx * fx * fx / (fx - equation->evaluate(xk - cx * fx)->getValue());

	if (abs(xk1 - xk) < pow(10, -precision)) {
	    root = xk1;
	    iterationCount = iterations;
	    return true;
	}
	xk = xk1;
	iterations++;
    }

    return false;
}

bool EquationSolver::solveUsingNewtonMethod(math::Entry* equation, math::Entry* derivative, double a, double b, int precision, double& root, int& iterationCount) {
    math::Operator* auxFunc = new math::SubtractFunction();
    auxFunc->addInput(new math::VariableEntry());
    math::Operator* divide = new math::DivideFunction();
    divide->addInput(equation);
    divide->addInput(derivative);
    auxFunc->addInput(divide);

    double xk = a;
    double xk1 = -1;
    double fx;
    int iterations = 0;
    while (true) {
        if (iterations > 100000) {
            return false;
        }
        xk1 = auxFunc->evaluate(xk)->getValue();
        fx = equation->evaluate(xk1)->getValue();

	if (abs(fx) < pow(10, -precision)) {
	    root = xk1;
	    iterationCount = iterations;
	    return true;
	}
	xk = xk1;
	iterations++;
    }

    return false;
}

bool EquationSolver::solveUsingDichotomy(math::Entry* equation, double a, double b, int precision, double& root, int& iterationCount) {
    double x, f;
    double left;
    double right;

    if (equation->evaluate(b)->getValue() > 0) {
        left = a;
        right = b;
    } else {
        left = b;
        right = a;
    }

    int iterations = 0;
    while (true) {
        if (iterations > 100000) {
            return false;
        }

	x = (left + right) / 2;
	f = equation->evaluate(x)->getValue();
	if (f > 0) {
	    right = x;
	} else {
	    left = x;
	}

	if (abs(f) < pow(10, -precision)) {
	    root = x;
	    iterationCount = iterations;
	    return true;
	}
	iterations++;
    }

    return false;
}
