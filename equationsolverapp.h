#ifndef EQUATIONSOLVERAPP_H
#define EQUATIONSOLVERAPP_H

#include <QDebug>
#include <QMainWindow>
#include <QUrl>
#include <QtWebEngineWidgets>

#include "window.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class EquationSolverApp;
}
QT_END_NAMESPACE

// Class resposible for main window and tabs
class EquationSolverApp : public QMainWindow {
    Q_OBJECT

public:
    EquationSolverApp(QWidget* parent = nullptr);
    ~EquationSolverApp();


private slots:
    void changeTabName(int index, QString newName);
    void onChangeTab(int index);
    void showContextMenu(const QPoint& pos);
    void closeTab();
    void newTab();
    void closeApp();
    void saveTabs();
    void loadSession();

private:
    void saveToFile(QVector<Window*> tabs, QString path);
    void loadFromFile(QString path);

private:
    bool newTabAllowed = true;
    Ui::EquationSolverApp* ui;
};
#endif  // EQUATIONSOLVERAPP_H
