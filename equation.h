#ifndef EQUATION_H
#define EQUATION_H
#include <string>
#include <vector>
#include "utils.h"

namespace math {

struct Tuple {
    double a;
    double b;

    Tuple(double a, double b);
};

class Interval {
public:
    std::vector<Tuple> entries;

    Interval(QString interval);
    Interval();

    bool isWithin(double number);
    void addEntry(Tuple entry);
    int size();
    Tuple getAt(int index);
};

// Main class holding equation tree
class Entry {
public:
    virtual ~Entry() = DEFAULT;

    // Traverse tree and evaluate function value at x
    virtual Entry* evaluate(double x) { return nullptr; }
    // get numberic value of constant or variable entries
    virtual double getValue() { return 0; }
    // full copy of the tree
    virtual Entry* copy() { return nullptr; }
    // does function contain variable x
    virtual bool isVariable() { return false; }
    // get analyticaly derivative
    virtual Entry* getDerivative() { return nullptr; }
    virtual std::string to_string(Entry const&) { return "Entry base"; }
};

class EquationHolder {
public:
    Entry* equation;
    Entry* derivative;
    bool windowReady = false;
};

class ConstantEntry : public Entry {
protected:
    double value;

public:
    ConstantEntry(double value);

    double getValue() override;

    void setValue(double value);

    Entry* evaluate(double x) override;

    Entry* copy() override;

    bool isVariable() override;

    Entry* getDerivative() override;

    std::string to_string(Entry const&) override;
};

class VariableEntry : public ConstantEntry {
public:
    VariableEntry();

    double getValue() override;

    Entry* evaluate(double x) override;

    Entry* copy() override;

    bool isVariable() override;

    Entry* getDerivative() override;

    std::string to_string(Entry const&) override;
};

class StringEntry : public Entry {
protected:
    std::string value;
    Entry* parsedValue;

public:
    StringEntry(std::string value);

    double getValue() override;

    Entry* evaluate(double x) override;

    Entry* copy() override;

    bool isVariable() override;

    Entry* getDerivative() override;

    std::string to_string(Entry const&) override;
};

// Operator class. This could be any defined function. All of them are defined below
class Operator : public Entry, public Base {
protected:
    std::vector<Entry*> input;

public:
    ~Operator() override {
        for (auto p : input) {
            delete p;
        }
    }

    virtual std::string getFunctionName() {
        return "";
    }

    std::string getType() override;

    virtual double function(std::vector<double> input) {
        return 0;
    }

    // minimum number of arguments accepted (default 1)
    virtual size_t acceptedArgsNumber() {
        return 1;
    }

    bool isVariable() override;

    virtual bool hasPriority() {
        return false;
    }

    void addInput(Entry* entry);

    double getValue() override;

    Entry* evaluate(double x) override;

    Entry* copy() override;

    std::string to_string(Entry const&) override;
};

class AddFunction : public Operator {
    std::string getFunctionName() override;

    size_t acceptedArgsNumber() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class SubtractFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class MultiplyFunction : public Operator {
    std::string getFunctionName() override;

    size_t acceptedArgsNumber() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;

    bool hasPriority() override;
};

class DivideFunction : public Operator {
    std::string getFunctionName() override;

    size_t acceptedArgsNumber() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;

    bool hasPriority() override;
};

class PowerFunction : public Operator {
    std::string getFunctionName() override;

    size_t acceptedArgsNumber() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class AbsFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class SignFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class SqrtFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class SinFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class CosFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class TanFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class CotFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class LnFunction : public Operator {
    std::string getFunctionName() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

class LogFunction : public Operator {
    std::string getFunctionName() override;

    size_t acceptedArgsNumber() override;

    double function(std::vector<double> input) override;

    Entry* getDerivative() override;
};

}  // namespace math

#endif  // EQUATION_H
