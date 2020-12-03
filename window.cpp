#include "window.h"
#include "ui_window.h"

#include <QMessageBox>

#include <cmath>
#include "equationparser.h"
#include "utils.h"


Window::~Window() {
    delete ui;
    view->close();
    delete view;
    delete plotter;
}

Window::Window(int index, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::Window) {
    ui->setupUi(this);

    myIndex = index;
    initWindow();
}

Window::Window(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::Window) {
    ui->setupUi(this);

    initWindow();
}

// Prepare form for display
void Window::initWindow() {
    view = new QWebEngineView(ui->formulaOutput);
    view->setContextMenuPolicy(Qt::PreventContextMenu);
    QString html = read(":/display.html");
    view->setHtml(html, QUrl("local file"));

    ui->formulaOutput->layout()->addWidget(view);
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    view->show();

    plotter = (QCustomPlot*)ui->graphs;

    plotter->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom));  // period as decimal separator and comma as thousand separator
    plotter->legend->setVisible(true);
    QFont legendFont = font();   // start out with MainWindow's font..
    legendFont.setPointSize(9);  // and make a bit smaller for legend
    plotter->legend->setFont(legendFont);
    plotter->legend->setBrush(QBrush(QColor(255, 255, 255, 230)));
    plotter->setAutoAddPlottableToLegend(false);
    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
    plotter->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignRight);

    // configure right and top axis to show ticks but no labels:
    plotter->xAxis2->setVisible(true);
    plotter->xAxis2->setTickLabels(false);
    plotter->yAxis2->setVisible(true);
    plotter->yAxis2->setTickLabels(false);
    plotter->xAxis->setRange(-10, 10);
    plotter->yAxis->setRange(-10, 10);
    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(plotter->xAxis, SIGNAL(rangeChanged(QCPRange)), plotter->xAxis2, SLOT(setRange(QCPRange)));
    connect(plotter->yAxis, SIGNAL(rangeChanged(QCPRange)), plotter->yAxis2, SLOT(setRange(QCPRange)));
    // Note: we could have also just called customPlot->rescaleAxes(); instead
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    plotter->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    connect(plotter, SIGNAL(beforeReplot()), this, SLOT(updateGraphs()));
}

#define PLOT_REZ 400


void Window::addGraph(bool derivative) {
    plotter->addGraph();
    if (derivative) {
        plotter->graph(plotter->graphCount() - 1)->setPen(QPen(Qt::blue));  // line color blue for second graph
        if (!hasDerivativeGraph) {
            plotter->graph(plotter->graphCount() - 1)->setName("Derivative");
            plotter->graph(plotter->graphCount() - 1)->addToLegend(plotter->legend);
            hasDerivativeGraph = true;
        }
    } else {
        plotter->graph(plotter->graphCount() - 1)->setPen(QPen(Qt::red));  // line color red for first graph
        if (!hasFunctionGraph) {
            plotter->graph(plotter->graphCount() - 1)->setName("Function");
            plotter->graph(plotter->graphCount() - 1)->addToLegend(plotter->legend);
            hasFunctionGraph = true;
        }
        QPen pen = QPen(Qt::red);
        pen.setWidth(2);
        plotter->graph(plotter->graphCount() - 1)->selectionDecorator()->setPen(pen);
    }
}

// Handle redraw of function graphs
void Window::updateGraphs() {
    if (windowReady) {
        plotter->clearGraphs();
        hasFunctionGraph = false;
        hasDerivativeGraph = false;
        QCPRange xrange = plotter->xAxis->range();
        QCPRange yrange = plotter->yAxis->range();

        int plotResolution = PLOT_REZ / 8 * xrange.size();

	// generate some points of data (y0 for first, y1 for second graph):
	QVector<double> x(plotResolution + 1), y0(plotResolution + 1), y1(plotResolution + 1);
	int lastContinius = 0;
	for (int i = 0; i <= plotResolution; ++i) {
	    x[i] = i * xrange.size() / plotResolution + xrange.lower;
	    y0[i] = equation->evaluate(x[i])->getValue();
	    y1[i] = derivative->evaluate(x[i])->getValue();

	    if (std::abs(y0[i] - yrange.center()) > 50 && i - lastContinius > 0) {
		QVector<double> xSegment(i - lastContinius + 1), y0Segment(i - lastContinius + 1), y1Segment(i - lastContinius + 1);
		for (int j = 0; j <= i - lastContinius; j++) {
		    xSegment[j] = x[j + lastContinius];
		    if (j == i - lastContinius) {
			y0Segment[j] = (double)(sign(y0[j + lastContinius - 1]) * 100000);
			y1Segment[j] = (double)(sign(y1[j + lastContinius - 1]) * 100000);
		    } else {
			y0Segment[j] = y0[j + lastContinius];
			y1Segment[j] = y1[j + lastContinius];
		    }
		}
		addGraph(false);
		plotter->graph(plotter->graphCount() - 1)->setData(xSegment, y0Segment);
		addGraph(true);
		plotter->graph(plotter->graphCount() - 1)->setData(xSegment, y1Segment);
		lastContinius = i + 1;
	    }
	}
	if (plotResolution - lastContinius + 1 > 0) {
	    QVector<double> xSegment(plotResolution - lastContinius + 1), y0Segment(plotResolution - lastContinius + 1), y1Segment(plotResolution - lastContinius + 1);
	    for (int j = 0; j <= plotResolution - lastContinius; j++) {
		xSegment[j] = x[j + lastContinius];
		y0Segment[j] = y0[j + lastContinius];
		y1Segment[j] = y1[j + lastContinius];
	    }
	    addGraph(false);
	    plotter->graph(plotter->graphCount() - 1)->setData(xSegment, y0Segment);
	    addGraph(true);
	    plotter->graph(plotter->graphCount() - 1)->setData(xSegment, y1Segment);
	}
    }
}
// Input and parse function equation
void Window::on_confirmButton_clicked() {
    windowReady = false;
    QString input = ui->equationInput->text();
    try {
        equation = EquationParser::parseEquation(input.toStdString(), 15);
        derivative = equation->getDerivative();
        emit tabNameChanged(myIndex, input);
    } catch (std::exception e) {
        QMessageBox::warning(this, "Warning", "Error parsing entered equation!\nPlease check your syntax.");
        qDebug() << "Error parsing your input!";
        return;
    }

    EquationParser::prepareForDisplay(input);
    view->page()->runJavaScript("updateEquation(\"f\\\\brac x = " + input + "\")");

    windowReady = true;
    plotter->replot();
}

void Window::on_solveButton_clicked() {
    if (windowReady) {
        double root = 0;
        try {
            ui->rootList->clear();
            int precision = ui->precision->value();
            QString intervalStr = ui->intervalInput->text();
            math::Interval* userInterval = new math::Interval(intervalStr);
            if (userInterval->size() == 0) {
                QMessageBox::warning(this, "Warning", "Entered interval is either empty or malformed!");
                return;
            }
            math::Interval* searchInterval;
            if (ui->doSearchForRoots->isChecked() && ui->searchStep->value() > 0) {
                int splitIterations = 0;
                searchInterval = EquationSolver::splitInterval(equation, userInterval, ui->searchStep->value(), splitIterations);

		ui->rootList->addItem(QString::fromStdString("Searching for roots took " + std::to_string(splitIterations) + " itterations"));
	    } else {
		searchInterval = userInterval;
	    }
	    bool result = false;
	    int itterations = 0;

	    // Go through each interval and seacrch for roots
	    for (int i = 0; i < searchInterval->size(); i++) {
		math::Tuple entry = searchInterval->getAt(i);

		int method = ui->tabWidget->currentIndex();
		switch (method) {
		    case 0: {
			bool fast = ui->fastIterationCB->isChecked();
			math::Entry* cFunc = EquationParser::parseEquation(ui->iterationFunctionField->text().toStdString(), 15);

			if (fast) {
			    result = EquationSolver::solveUsingFastItterations(equation, cFunc, entry.a, entry.b, precision, root, itterations);
			} else {
			    result = EquationSolver::solveUsingSimpleItterations(equation, cFunc, entry.a, entry.b, precision, root, itterations);
			}
			break;
		    }
		    case 1: {
			result = EquationSolver::solveUsingNewtonMethod(equation, derivative, entry.a, entry.b, precision, root, itterations);
			break;
		    }
		    case 2: {
			result = EquationSolver::solveUsingDichotomy(equation, entry.a, entry.b, precision, root, itterations);
			break;
		    }
		}

		if (result) {
		    ui->rootList->addItem(QString::fromStdString("Root: x= " + std::to_string(root) + " after " + std::to_string(itterations) + " itterations"));
		}
	    }
	} catch (std::exception e) {
	    QMessageBox::warning(this, "Warning", "Error parsing entered itteration function!\nPlease check your syntax.");
	    qDebug() << "Error parsing your input!";
	    return;
	}
    }
}

// handling saving and loading of state

QJsonObject Window::writeToJson() {
    QJsonObject json;

    json["equation"] = ui->equationInput->text();
    json["precision"] = ui->precision->value();
    json["interval"] = ui->intervalInput->text();
    json["doSearch"] = ui->doSearchForRoots->isChecked();
    json["searchStep"] = ui->searchStep->value();
    json["method"] = ui->tabWidget->currentIndex();
    if (ui->tabWidget->currentIndex() == 0) {
        json["cfunction"] = ui->iterationFunctionField->text();
        bool fast = ui->fastIterationCB->isChecked();
        json["usingFast"] = fast;
    }
    QJsonArray array;
    for (int i = 0; i < ui->rootList->count(); ++i) {
        array.append(ui->rootList->item(i)->text());
    }
    json["roots"] = array;

    return json;
}


void Window::readFromJson(QJsonObject json) {
    ui->equationInput->setText(json["equation"].toString());
    ui->precision->setValue(json["precision"].toInt());
    ui->intervalInput->setText(json["interval"].toString());
    ui->doSearchForRoots->setChecked(json["doSearch"].toBool());
    ui->searchStep->setValue(json["searchStep"].toDouble());
    ui->tabWidget->setCurrentIndex(json["method"].toInt());
    if (json["method"].toInt() == 0) {
        ui->iterationFunctionField->setText(json["cfunction"].toString());
        ui->fastIterationCB->setChecked(json["usingFast"].toBool());
    }
    QJsonArray array = json["roots"].toArray();
    for (int i = 0; i < array.size(); ++i) {
        ui->rootList->addItem(array.at(i).toString());
    }

    QTimer::singleShot(2500, this, SLOT(on_confirmButton_clicked()));
}
