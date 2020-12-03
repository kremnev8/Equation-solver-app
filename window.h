#ifndef WINDOW_H
#define WINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QUrl>
#include <QWidget>
#include <QtWebEngineWidgets>

#include "equation.h"
#include "equationsolver.h"
#include "qcustomplot.h"
#include "utils.h"

namespace Ui {
class Window;
}
// Class responsible for equation solving form.
class Window : public QWidget, public math::EquationHolder {
    Q_OBJECT

public:
    explicit Window(QWidget* parent = nullptr);
    Window(int index, QWidget* parent = nullptr);
    ~Window();

    QJsonObject writeToJson();
    void readFromJson(QJsonObject json);

private:
    void addGraph(bool derivative);
    void initWindow();

public slots:
    void on_confirmButton_clicked();

    void on_solveButton_clicked();

    void updateGraphs();

signals:
    void tabNameChanged(int index, QString newValue);

private:
    QWebEngineView* view;
    QCustomPlot* plotter;
    bool hasFunctionGraph;
    bool hasDerivativeGraph;
    int myIndex;

private:
    Ui::Window* ui;
};

#endif  // WINDOW_H
