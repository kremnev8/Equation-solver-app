#include "equationsolverapp.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    EquationSolverApp w;
    w.show();
    return a.exec();
}
