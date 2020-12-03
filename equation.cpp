#include "equation.h"
#include "equationparser.h"

namespace math {

// Filling registry with known functions
namespace {
ConcreteFactory<math::AddFunction> factoryAdd("+");
ConcreteFactory<math::AddFunction> factoryAdd1("add");
ConcreteFactory<math::SubtractFunction> factorySub("-");
ConcreteFactory<math::SubtractFunction> factorySub1("sub");

ConcreteFactory<math::MultiplyFunction> factoryMul("*");
ConcreteFactory<math::MultiplyFunction> factoryMul1("times");
ConcreteFactory<math::MultiplyFunction> factoryMul2("mul");

ConcreteFactory<math::DivideFunction> factoryDiv("/");
ConcreteFactory<math::DivideFunction> factoryDiv1("div");
ConcreteFactory<math::DivideFunction> factoryDiv2("frac");

ConcreteFactory<math::PowerFunction> factoryPow("pow");
ConcreteFactory<math::SqrtFunction> factorySqrt("sqrt");

ConcreteFactory<math::SignFunction> factorySign("sign");
ConcreteFactory<math::AbsFunction> factoryAbs("abs");

ConcreteFactory<math::SinFunction> factorySin("sin");
ConcreteFactory<math::CosFunction> factoryCos("cos");
ConcreteFactory<math::TanFunction> factoryTan("tan");
ConcreteFactory<math::CotFunction> factoryCot("cot");

ConcreteFactory<math::LnFunction> factoryLn("ln");
ConcreteFactory<math::LogFunction> factoryLog("log");
}  // namespace


Tuple::Tuple(double a, double b) {
    this->a = a;
    this->b = b;
}

bool Interval::isWithin(double number) {
    for (size_t i = 0; i < entries.size(); i++) {
        Tuple entry = entries.at(i);
        if (entry.a <= number && number <= entry.b) {
            return true;
        }
    }
    return false;
}

Interval::Interval(QString interval) {
    interval.replace("(", "");
    interval.replace(")", "");
    interval.replace("[", "");
    interval.replace("]", "");
    QStringList list = interval.split("&");
    for (int i = 0; i < list.size(); i++) {
        QString entryStr = list.at(i);
        QStringList entryValues = entryStr.split(";");
        if (entryValues.size() > 1) {
            double a = std::stod(entryValues.at(0).toStdString());
            double b = std::stof(entryValues.at(1).toStdString());
            if (a < b) {
                Tuple entry = Tuple(a, b);
                addEntry(entry);
            }
        }
    }
}

Interval::Interval() {
}

void Interval::addEntry(Tuple entry) {
    entries.push_back(entry);
}

int Interval::size() {
    return entries.size();
}

Tuple Interval::getAt(int index) {
    return entries.at(index);
}

ConstantEntry::ConstantEntry(double value) {
    this->value = value;
}

double ConstantEntry::getValue() {
    return value;
}

void ConstantEntry::setValue(double value) {
    this->value = value;
}

Entry* ConstantEntry::evaluate(double x) {
    return this;
}

Entry* ConstantEntry::copy() {
    return new ConstantEntry(value);
}

bool ConstantEntry::isVariable() {
    return false;
}

Entry* ConstantEntry::getDerivative() {
    return new ConstantEntry(0);
}

std::string ConstantEntry::to_string(Entry const&) {
    return std::to_string(value);
}

VariableEntry::VariableEntry()
    : ConstantEntry(0) {
}

double VariableEntry::getValue() {
    return value;
}

Entry* VariableEntry::evaluate(double x) {
    value = x;
    return this;
}

Entry* VariableEntry::copy() {
    return new VariableEntry();
}

Entry* VariableEntry::getDerivative() {
    return new ConstantEntry(1);
}


bool VariableEntry::isVariable() {
    return true;
}

std::string VariableEntry::to_string(Entry const&) {
    return "x";
}

StringEntry::StringEntry(std::string value) {
    this->value = value;
    this->parsedValue = EquationParser::parseEquation(value, 15);
}

double StringEntry::getValue() {
    return parsedValue->getValue();
}

Entry* StringEntry::evaluate(double x) {
    return parsedValue;
}

Entry* StringEntry::copy() {
    return parsedValue->copy();
}

Entry* StringEntry::getDerivative() {
    return parsedValue->getDerivative();
}


bool StringEntry::isVariable() {
    return parsedValue->isVariable();
}

std::string StringEntry::to_string(Entry const&) {
    return value;
}

std::string Operator::getType() {
    if (getFunctionName() != "") {
        return getFunctionName();
    }
    return "Base";
}

void Operator::addInput(Entry* entry) {
    this->input.push_back(entry);
}

double Operator::getValue() {
    return 0;
}

Entry* Operator::evaluate(double x) {
    if (input.size() < acceptedArgsNumber()) {
        qDebug() << "Function " << QString::fromStdString(this->getFunctionName()) << " got " << input.size() << " which is less than " << acceptedArgsNumber();
        return new ConstantEntry(0);
    }

    std::vector<double> inputVal(input.size());

    for (size_t i = 0; i < input.size(); i++) {
        inputVal.at(i) = input.at(i)->evaluate(x)->getValue();
    }

    return new ConstantEntry(function(inputVal));
}


Entry* Operator::copy() {
    Operator* copy = dynamic_cast<math::Operator*>(Factory::makeRaw(this->getType()));
    for (size_t i = 0; i < input.size(); i++) {
        copy->addInput(input.at(i)->copy());
    }
    return copy;
}

bool Operator::isVariable() {
    for (size_t i = 0; i < input.size(); i++) {
        if (input.at(i)->isVariable()) {
            return true;
        }
    }
    return false;
}

std::string Operator::to_string(Entry const&) {
    return getType();
}

std::string AddFunction::getFunctionName() {
    return "add";
}

size_t AddFunction::acceptedArgsNumber() {
    return 2;
}

double AddFunction::function(std::vector<double> input) {
    double acc = 0;
    for (size_t i = 0; i < input.size(); i++) {
        acc += input.at(i);
    }
    return acc;
}

Entry* AddFunction::getDerivative() {
    Operator* add = new AddFunction();
    for (size_t i = 0; i < input.size(); i++) {
        add->addInput(input.at(i)->getDerivative());
    }

    return add;
}

std::string SubtractFunction::getFunctionName() {
    return "sub";
}

double SubtractFunction::function(std::vector<double> input) {
    if (input.size() == 2) {
        return input.at(0) - input.at(1);
    } else if (input.size() == 1) {
        return -input.at(0);
    }
    return 0;
}

Entry* SubtractFunction::getDerivative() {
    if (input.size() == 2) {
        Operator* sub = new SubtractFunction();
        sub->addInput(input.at(0)->getDerivative());
        sub->addInput(input.at(1)->getDerivative());
        return sub;
    } else if (input.size() == 1) {
        Operator* mul = new SubtractFunction();
        mul->addInput(input.at(0)->getDerivative());
        mul->addInput(new ConstantEntry(-1));
        return mul;
    }
    return new ConstantEntry(0);
}

std::string MultiplyFunction::getFunctionName() {
    return "mul";
}

size_t MultiplyFunction::acceptedArgsNumber() {
    return 2;
}

double MultiplyFunction::function(std::vector<double> input) {
    double acc = 1;
    for (size_t i = 0; i < input.size(); i++) {
        acc *= input.at(i);
    }
    return acc;
}

Entry* MultiplyFunction::getDerivative() {
    if (input.size() == 2) {
        if (input.at(0)->isVariable() && input.at(1)->isVariable()) {
            Operator* mul1 = new MultiplyFunction();
            mul1->addInput(input.at(0)->getDerivative());
            mul1->addInput(input.at(1)->copy());
            Operator* mul2 = new MultiplyFunction();
            mul2->addInput(input.at(0)->copy());
            mul2->addInput(input.at(1)->getDerivative());
            Operator* add = new AddFunction();
            add->addInput(mul1);
            add->addInput(mul2);
            return add;
        } else if (input.at(0)->isVariable()) {
            Operator* mul = new MultiplyFunction();
            mul->addInput(input.at(0)->getDerivative());
            mul->addInput(input.at(1)->copy());
            return mul;
        } else if (input.at(1)->isVariable()) {
            Operator* mul = new MultiplyFunction();
            mul->addInput(input.at(1)->getDerivative());
            mul->addInput(input.at(0)->copy());
            return mul;
        }
        return new ConstantEntry(0);
    }
    return new ConstantEntry(0);
}

bool MultiplyFunction::hasPriority() {
    return true;
}

std::string DivideFunction::getFunctionName() {
    return "div";
}

size_t DivideFunction::acceptedArgsNumber() {
    return 2;
}

double DivideFunction::function(std::vector<double> input) {
    return input.at(0) / input.at(1);
}

Entry* DivideFunction::getDerivative() {
    if (input.size() == 2) {
        if (input.at(0)->isVariable() && input.at(1)->isVariable()) {
            Operator* mul1 = new MultiplyFunction();
            mul1->addInput(input.at(0)->getDerivative());
            mul1->addInput(input.at(1)->copy());
            Operator* mul2 = new MultiplyFunction();
            mul2->addInput(input.at(0)->copy());
            mul2->addInput(input.at(1)->getDerivative());
            Operator* sub = new SubtractFunction();
            sub->addInput(mul1);
            sub->addInput(mul2);
            Operator* pow = new PowerFunction();
            pow->addInput(input.at(1)->copy());
            pow->addInput(new ConstantEntry(2));
            Operator* div = new DivideFunction();
            div->addInput(sub);
            div->addInput(pow);
            return div;
        } else if (input.at(0)->isVariable()) {
            Operator* div = new DivideFunction();
            div->addInput(input.at(0)->getDerivative());
            div->addInput(input.at(1)->copy());
            return div;
        } else if (input.at(1)->isVariable()) {
            Operator* pow = new PowerFunction();
            pow->addInput(input.at(1)->copy());
            pow->addInput(new ConstantEntry(-1));
            Entry* diriv = pow->getDerivative();
            Operator* mul = new MultiplyFunction();
            mul->addInput(diriv);
            mul->addInput(input.at(0)->copy());
            delete pow;
            return mul;
        }
        return new ConstantEntry(0);
    }
    return new ConstantEntry(0);
}

bool DivideFunction::hasPriority() {
    return true;
}

std::string PowerFunction::getFunctionName() {
    return "pow";
}

size_t PowerFunction::acceptedArgsNumber() {
    return 2;
}

double PowerFunction::function(std::vector<double> input) {
    return powf(input.at(0), input.at(1));
}

Entry* PowerFunction::getDerivative() {
    if (input.size() == 2) {
        if (input.at(0)->isVariable() && input.at(1)->isVariable()) {
            Operator* pow = dynamic_cast<Operator*>(copy());

	    Operator* mul = new MultiplyFunction();
	    mul->addInput(input.at(1)->copy());
	    Operator* ln = new LnFunction();
	    ln->addInput(input.at(0)->copy());
	    mul->addInput(ln);
	    Operator* mul1 = new MultiplyFunction();
	    mul1->addInput(pow);
	    mul1->addInput(mul->getDerivative());
	    return mul1;
	} else if (input.at(0)->isVariable()) {
	    Operator* mul = new MultiplyFunction();
	    Entry* power = input.at(1)->copy();
	    mul->addInput(power);
	    mul->addInput(input.at(0)->getDerivative());
	    Operator* mul1 = new MultiplyFunction();
	    mul1->addInput(mul);
	    Operator* pow = new PowerFunction();
	    pow->addInput(input.at(0)->copy());
	    Operator* sub = new SubtractFunction();
	    sub->addInput(power);
	    sub->addInput(new ConstantEntry(1));
	    pow->addInput(sub);
	    mul1->addInput(pow);
	    return mul1;
	} else if (input.at(1)->isVariable()) {
	    Operator* pow = dynamic_cast<Operator*>(copy());

	    Operator* mul = new MultiplyFunction();
	    mul->addInput(pow);
	    Operator* ln = new LnFunction();
	    ln->addInput(input.at(0)->copy());
	    mul->addInput(ln);
	    Operator* mul1 = new MultiplyFunction();
	    mul1->addInput(mul);
	    mul1->addInput(input.at(1)->getDerivative());
	    return mul1;
	}
	return new ConstantEntry(0);
    }
    return new ConstantEntry(0);
}

std::string SignFunction::getFunctionName() {
    return "sign";
};

double SignFunction::function(std::vector<double> input) {
    return (0 < input.at(0)) - (input.at(0) < 0);
};

Entry* SignFunction::getDerivative() {
    return new ConstantEntry(0);
};

std::string AbsFunction::getFunctionName() {
    return "abs";
}

double AbsFunction::function(std::vector<double> input) {
    return abs(input.at(0));
}

Entry* AbsFunction::getDerivative() {
    Operator* sign = new SignFunction();
    sign->addInput(input.at(0)->copy());
    Operator* mul = new MultiplyFunction();
    mul->addInput(sign);
    mul->addInput(input.at(0)->getDerivative());

    return mul;
}

std::string SqrtFunction::getFunctionName() {
    return "sqrt";
}

double SqrtFunction::function(std::vector<double> input) {
    return sqrt(input.at(0));
}

Entry* SqrtFunction::getDerivative() {
    Operator* pow = new PowerFunction();
    pow->addInput(input.at(0)->copy());
    pow->addInput(new ConstantEntry(0.5));

    return pow->getDerivative();
}

std::string SinFunction::getFunctionName() {
    return "sin";
}

double SinFunction::function(std::vector<double> input) {
    return sinf(input.at(0));
}

Entry* SinFunction::getDerivative() {
    Operator* cos = new CosFunction();
    cos->addInput(input.at(0)->copy());
    Operator* mul = new MultiplyFunction();
    mul->addInput(cos);
    mul->addInput(input.at(0)->getDerivative());

    return mul;
}

std::string CosFunction::getFunctionName() {
    return "cos";
}

double CosFunction::function(std::vector<double> input) {
    return cosf(input.at(0));
}

Entry* CosFunction::getDerivative() {
    Operator* sin = new SinFunction();
    sin->addInput(input.at(0)->copy());
    Operator* mul = new MultiplyFunction();
    mul->addInput(sin);
    mul->addInput(new ConstantEntry(-1));
    Operator* mul1 = new MultiplyFunction();
    mul1->addInput(mul);
    mul1->addInput(input.at(0)->getDerivative());

    return mul1;
}

std::string TanFunction::getFunctionName() {
    return "tan";
}

double TanFunction::function(std::vector<double> input) {
    return tanf(input.at(0));
}

Entry* TanFunction::getDerivative() {
    Operator* cos = new CosFunction();
    cos->addInput(input.at(0)->copy());
    Operator* pow = new PowerFunction();
    pow->addInput(cos);
    pow->addInput(new ConstantEntry(2));
    Operator* div = new DivideFunction();
    div->addInput(input.at(0)->getDerivative());
    div->addInput(pow);

    return div;
}

std::string CotFunction::getFunctionName() {
    return "cot";
}

double CotFunction::function(std::vector<double> input) {
    return 1 / tanf(input.at(0));
}

Entry* CotFunction::getDerivative() {
    Operator* sin = new SinFunction();
    sin->addInput(input.at(0)->copy());
    Operator* pow = new PowerFunction();
    pow->addInput(sin);
    pow->addInput(new ConstantEntry(2));
    Operator* div = new DivideFunction();
    div->addInput(input.at(0)->getDerivative());
    div->addInput(pow);
    Operator* mul = new MultiplyFunction();
    mul->addInput(div);
    mul->addInput(new ConstantEntry(-1));

    return mul;
}

std::string LnFunction::getFunctionName() {
    return "ln";
}

double LnFunction::function(std::vector<double> input) {
    return logf(input.at(0));
}

Entry* LnFunction::getDerivative() {
    Operator* div = new DivideFunction();
    div->addInput(input.at(0)->getDerivative());
    div->addInput(input.at(0)->copy());

    return div;
}

std::string LogFunction::getFunctionName() {
    return "log";
}

size_t LogFunction::acceptedArgsNumber() {
    return 2;
}

double LogFunction::function(std::vector<double> input) {
    if (input.size() == 2) {
        return logf(input.at(1)) / logf(input.at(0));
    }
    return 0;
}

Entry* LogFunction::getDerivative() {
    Operator* div = new DivideFunction();
    div->addInput(input.at(1)->getDerivative());
    Operator* mul = new MultiplyFunction();
    mul->addInput(input.at(1)->copy());
    Operator* ln = new LnFunction();
    ln->addInput(input.at(0));
    mul->addInput(ln);
    div->addInput(mul);

    return div;
}

}  // namespace math
