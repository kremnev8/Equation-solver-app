#include "equationsolverapp.h"
#include <QApplication>
#include <QChar>
#include <algorithm>
#include "ui_equationsolverapp.h"
#include "window.h"

EquationSolverApp::EquationSolverApp(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::EquationSolverApp) {
    ui->setupUi(this);

    ui->tabWidget->addTab(new QLabel("+"), QString("+"));
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &EquationSolverApp::onChangeTab);
    newTab();

    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenu(const QPoint&)));

    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(newTab()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(closeApp()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveTabs()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(loadSession()));
}

EquationSolverApp::~EquationSolverApp() {
    newTabAllowed = false;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
    while (ui->tabWidget->count() >= 0) {
        closeTab();
    }
    delete ui;
}

void EquationSolverApp::closeApp() {
    exit(0);
}

void EquationSolverApp::loadSession() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open sesson results"), "",
                                                    tr("Json file (*.json);;All Files (*)"));

    loadFromFile(fileName);
}

void EquationSolverApp::onChangeTab(int index) {
    if (index == this->ui->tabWidget->count() - 1) {
        newTab();
    }
}

void EquationSolverApp::changeTabName(int index, QString newName) {
    if (newName.size() > 15) {
        newName.truncate(15);
        newName += "...";
    }
    ui->tabWidget->setTabText(index, newName);
}

void EquationSolverApp::newTab() {
    if (newTabAllowed) {
        int position = ui->tabWidget->count() - 1;
        Window* window = new Window(position);
        ui->tabWidget->insertTab(position, window, QString("Solver tab"));

	connect(window, SIGNAL(tabNameChanged(int, QString)),
		this, SLOT(changeTabName(int, QString)));

	ui->tabWidget->setCurrentIndex(position);
	auto tabBar = ui->tabWidget->tabBar();
	tabBar->scroll(tabBar->width(), 0);
    }
}

void EquationSolverApp::showContextMenu(const QPoint& pos) {
    QMenu contextMenu(tr("Context menu"), this);

    QAction action1("Close tab", this);
    connect(&action1, SIGNAL(triggered()), this, SLOT(closeTab()));
    contextMenu.addAction(&action1);

    contextMenu.exec(mapToGlobal(pos));
}

void EquationSolverApp::closeTab() {
    int removeIndex = ui->tabWidget->currentIndex();
    if (ui->tabWidget->count() > 1 && removeIndex >= 1) {
        ui->tabWidget->setCurrentIndex(removeIndex - 1);
    }
    QWidget* page = ui->tabWidget->widget(removeIndex);
    ui->tabWidget->removeTab(removeIndex);
    delete page;
}

void EquationSolverApp::saveTabs() {
    QVector<Window*> tabs;
    for (int i = 0; i < ui->tabWidget->count() - 1; i++) {
        tabs.push_back(dynamic_cast<Window*>(ui->tabWidget->widget(i)));
    }
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save current sesson results"), "",
                                                    tr("Json file (*.json);;All Files (*)"));
    if (!fileName.isEmpty()) {
        saveToFile(tabs, fileName);
    }
}

void EquationSolverApp::saveToFile(QVector<Window*> tabs, QString path) {
    QFile* f = new QFile(path);
    if (f->open(QIODevice::WriteOnly)) {
        QJsonArray array;

	for (int i = 0; i < tabs.size(); i++) {
	    QJsonObject json = tabs.at(i)->writeToJson();
	    array.append(json);
	}

	QJsonDocument doc(array);
	f->write(doc.toJson());

        f->close();
    }
}

void EquationSolverApp::loadFromFile(QString path) {
    QFile* f = new QFile(path);
    if (f->open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(f->readAll());

	QJsonArray array = doc.array();
	for (int i = 0; i < array.size(); i++) {
	    int pos = ui->tabWidget->count() - 1;
	    newTab();
	    dynamic_cast<Window*>(ui->tabWidget->widget(pos))->readFromJson(array.at(i).toObject());
	}
    }
}
