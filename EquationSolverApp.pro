QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += webenginewidgets printsupport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = EquationSolverApp
TEMPLATE = app

SOURCES += \
    equation.cpp \
    equationparser.cpp \
    equationsolver.cpp \
    main.cpp \
    equationsolverapp.cpp \
    qcustomplot.cpp \
    utils.cpp \
    window.cpp

HEADERS += \
    equation.h \
    equationparser.h \
    equationsolver.h \
    equationsolverapp.h \
    qcustomplot.h \
    utils.h \
    window.h

FORMS += \
    equationsolverapp.ui \
    window.ui

RESOURCES += \
    resources.qrc

