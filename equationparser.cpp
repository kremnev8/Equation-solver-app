#include <qmath.h>

#include <QDebug>

#include "equationparser.h"


std::vector<std::string> split(std::string input, std::string delimiter) {
    std::vector<std::string> tokens;

    size_t pos = 0;
    std::string token;
    while ((pos = input.find(delimiter)) != std::string::npos) {
        token = input.substr(0, pos);
        tokens.push_back(token);
        input.erase(0, pos + delimiter.length());
    }
    tokens.push_back(input);
    return tokens;
}

template <class _Fn>
std::function<float(std::vector<float>)> unary(const _Fn& _Func) {
    return [&_Func](std::vector<float> array) -> float {
        if (array.size() == 1) {
            return _Func(array.at(0));
        }
        return 0;
    };
}

std::string EquationParser::findBrace(std::string& input, size_t start, int& end, char open, char close) {
    for (size_t i = start; i < input.size(); i++) {
        char ch = input.at(i);
        if (ch == open) {
            int endPos = -1;
            int braceIgnoreCount = 0;
            for (size_t j = i + 1; j < input.size(); j++) {
                char ch1 = input.at(j);
                if (ch1 == close) {
                    if (braceIgnoreCount == 0) {
                        endPos = j;
                        break;
                    } else {
                        braceIgnoreCount--;
                    }
                } else if (ch1 == open) {
                    braceIgnoreCount++;
                }
            }
            if (endPos != -1) {
                end = endPos + 1;
                return input.substr(i + 1, endPos - i - 1);
            }
        }
    }
    return input;
}

void EquationParser::prepareForDisplay(QString& input) {
    input.replace("\\sin", "\\sin\\brac");
    input.replace("\\cos", "\\cos\\brac");
    input.replace("\\tan", "\\tan\\brac");
    input.replace("\\cot", "\\cot\\brac");
    input.replace("\\ln", "\\ln\\brac");

    int pos = 0;
    while ((pos = input.indexOf("\\log_", pos)) != -1) {
        for (int i = pos; i < input.size(); i++) {
            QChar ch = input.at(i);
            if (ch == '{') {
                int endPos = -1;
                int braceIgnoreCount = 0;
                for (int j = i + 1; j < input.size(); j++) {
                    QChar ch1 = input.at(j);
                    if (ch1 == '}') {
                        if (braceIgnoreCount == 0) {
                            endPos = j;
                            break;
                        } else {
                            braceIgnoreCount--;
                        }
                    } else if (ch1 == '{') {
                        braceIgnoreCount++;
                    }
                }
                if (endPos != -1) {
                    input.insert(endPos + 1, "\\brac");
                    break;
                }
            }
        }
        pos += 5;
    }

    input.replace("\\", "\\\\");
}

int findEndOfName(std::string& input, std::function<bool(char)> predicate, int startPos = 0, int failValue = -1) {
    int endPos = failValue;
    for (size_t j = startPos; j < input.size(); j++) {
        char ch = input.at(j);
        if (predicate(ch)) {
            endPos = j;
            break;
        }
    }
    return endPos;
}

QDebug operator<<(QDebug out, const std::string& str) {
    out << QString::fromStdString(str);
    return out;
}


math::Entry* EquationParser::parseEquation(std::string input, int depth) {
    std::vector<math::Entry*> tokens;

    if (depth < 0) {
        qDebug() << "Depth limit reached, can't parse string!";
        throw std::invalid_argument("Reached depth limit, invalid input!");
    }

    // start with +-*/ and \div, \times
    int level = 0;
    int lastOp = 0;
    for (size_t i = 0; i < input.size(); i++) {
        char ch = input.at(i);
        switch (ch) {
            case '{':
            case '(': {
                level++;
                break;
            }
            case ')':
            case '}': {
                level--;
                break;
            }
            case '\\': {
                if (level != 0)
                    continue;
                int endPos = findEndOfName(
                    input, [](char c) {
                        return c == ' ' || c == '{' || c == '\\' || c == '+' || c == '-';
                    },
                    i + 1);
                if (endPos != -1) {
                    std::string name = input.substr(i + 1, endPos - i - 1);
                    if (name == "times" || name == "div") {
                        std::string left = input.substr(lastOp, i - lastOp);
                        std::string op = input.substr(i + 1, endPos - i - 1);
                        tokens.push_back(new math::StringEntry(left));
                        tokens.push_back(dynamic_cast<math::Entry*>(Factory::makeRaw(op)));
                        lastOp = endPos;
                        i = endPos - 1;
                    } else {
                        i = endPos - 1;
                    }
                }
                break;
            }
            case '-': {
                if (isdigit(input.at(i + 1))) {
                    break;
                }
                // fallthrough
            }
            case '*':
            case '/':
            case '+': {
                if (level != 0)
                    continue;
                std::string left = input.substr(lastOp, i - lastOp);
                std::string op = input.substr(i, 1);
                tokens.push_back(new math::StringEntry(left));
                tokens.push_back(dynamic_cast<math::Entry*>(Factory::makeRaw(op)));
                lastOp = i + 1;
            }
        }
    }

    // if found none, try to select a function inside the string
    if (lastOp == 0) {
        int start = findEndOfName(
            input, [](char c) {
                return c != ' ';
            },
            0, 0);
        // Evaluate contents of () braces
	if (input.at(start) == '(') {
	    return parseEquation(findBrace(input, start, start, '(', ')'), depth - 1);
	    // Find functions
	} else if (input.at(start) == '\\') {
	    int endPos = findEndOfName(
		input, [](char c) {
		    return c == ' ' || c == '{' || c == '\\' || c == '+' || c == '-';
		},
		start + 1);

	    std::string name;
	    if (endPos != -1) {
		name = input.substr(start + 1, endPos - start - 1);
	    } else {
		name = input.substr(start + 1, input.size() - start - 1);
	    }
	    // case of frac, log
	    if (name == "frac") {
		math::Operator* fracOp = dynamic_cast<math::Operator*>(Factory::makeRaw(name));
		fracOp->addInput(parseEquation(findBrace(input, endPos, endPos, '{', '}'), depth - 1));
		fracOp->addInput(parseEquation(findBrace(input, endPos, endPos, '{', '}'), depth - 1));
		return fracOp;
	    } else if (name == "log_") {
		math::Operator* logOp = dynamic_cast<math::Operator*>(Factory::makeRaw("log"));
		logOp->addInput(parseEquation(findBrace(input, endPos, endPos, '{', '}'), depth - 1));
		logOp->addInput(parseEquation(findBrace(input, endPos, endPos, '{', '}'), depth - 1));
		return logOp;
	    } else if (name == "pi") {
		return new math::ConstantEntry(M_PI);
		// generic case for a function
	    } else {
		math::Operator* op = dynamic_cast<math::Operator*>(Factory::makeRaw(name));
		if (op != nullptr) {
		    op->addInput(parseEquation(findBrace(input, endPos, endPos, '{', '}'), depth - 1));
		    return op;
		} else {
		    throw std::invalid_argument("No such function found!");
		}
	    }
	} else {
	    int start = findEndOfName(
		input, [](char c) {
		    return c != ' ';
		},
		0, 0);
	    // Find ^ operator
	    std::vector<std::string> tokens = split(input, "^");

	    if (tokens.size() > 1) {
		math::Operator* pow = dynamic_cast<math::Operator*>(Factory::makeRaw("pow"));
		if (tokens.at(0).at(0) == '{') {
		    int tmp = 0;
		    tokens[0] = findBrace(tokens.at(0), 0, tmp, '{', '}');
		}
		if (tokens.at(1).at(0) == '{') {
		    int tmp = 0;
		    tokens[1] = findBrace(tokens.at(1), 0, tmp, '{', '}');
		}
		pow->addInput(parseEquation(tokens.at(0), depth - 1));
		pow->addInput(parseEquation(tokens.at(1), depth - 1));
		return pow;
		// Final case. e and x.
	    } else if (input.at(start) == 'e') {
		return new math::ConstantEntry(M_E);
	    } else if (input.at(start) == 'x') {
		return new math::VariableEntry();
	    } else {
		return new math::ConstantEntry(std::stof(input));
	    }
	}
    }
    // if found +-*/ then parse each operand with respect to priority rules
    std::string last = input.substr(lastOp, input.size() - lastOp);
    tokens.push_back(new math::StringEntry(last));

    for (size_t i = 1; i < tokens.size(); i = i + 2) {
        math::Operator* func = dynamic_cast<math::Operator*>(tokens.at(i));
        if (func->hasPriority()) {
            func->addInput(tokens.at(i - 1)->copy());
            func->addInput(tokens.at(i + 1)->copy());
            tokens.erase(tokens.begin() + (i - 1), tokens.begin() + (i + 1));
            tokens[i - 1] = func;
            i = i - 2;
        }
    }

    math::Entry* left = tokens.at(0)->copy();
    tokens.erase(tokens.begin());

    while (tokens.size() > 1) {
        math::Operator* func = dynamic_cast<math::Operator*>(tokens.at(0));
        func->addInput(left);
        func->addInput(tokens.at(1)->copy());
        tokens.erase(tokens.begin(), tokens.begin() + 2);
        left = func;
    }

    return left;
}
