#include "mainwindow.h"

#include "backend/io/GraphLoader.h"
#include "backend/algorithms/AMCMCPEP.h"
#include "backend/algorithms/AMCPEBCP.h"
#include "backend/algorithms/ParametricSearch.h"

#include "frontend/ControlPanelWidget.h"
#include "frontend/EdgeDetailsWidget.h"
#include "frontend/GenerateGraphWidget.h"
#include "frontend/GraphWidget.h"
#include "frontend/ResultWidget.h"
#include "frontend/SidebarWidget.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    buildUi();
    applyStyle();
    refreshViewState();
}

void MainWindow::buildUi()
{
    setWindowTitle("Network Bandwidth Optimizer");
    resize(1320, 820);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    sidebar_ = new SidebarWidget(this);
    stackedWidget_ = new QStackedWidget(this);

    generateGraphWidget_ = new GenerateGraphWidget(this);

    loadRunPage_ = new QWidget(this);
    controls_ = new ControlPanelWidget(loadRunPage_);
    graphWidget_ = new GraphWidget(loadRunPage_);
    edgeDetails_ = new EdgeDetailsWidget(loadRunPage_);
    resultWidget_ = new ResultWidget(loadRunPage_);

    auto* centerLayout = new QVBoxLayout();
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(12);
    centerLayout->addWidget(controls_);
    centerLayout->addWidget(graphWidget_, 1);
    centerLayout->addWidget(resultWidget_);

    auto* loadRunLayout = new QHBoxLayout(loadRunPage_);
    loadRunLayout->setContentsMargins(0, 0, 0, 0);
    loadRunLayout->setSpacing(12);
    loadRunLayout->addLayout(centerLayout, 1);
    loadRunLayout->addWidget(edgeDetails_);

    stackedWidget_->addWidget(generateGraphWidget_);
    stackedWidget_->addWidget(loadRunPage_);
    stackedWidget_->setCurrentWidget(loadRunPage_);

    auto* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(12);
    rootLayout->addWidget(sidebar_);
    rootLayout->addWidget(stackedWidget_, 1);

    connect(sidebar_, &SidebarWidget::generateGraphClicked, this, [this]() {
        stackedWidget_->setCurrentWidget(generateGraphWidget_);
        });

    connect(sidebar_, &SidebarWidget::loadRunClicked, this, [this]() {
        stackedWidget_->setCurrentWidget(loadRunPage_);
        });

    connect(generateGraphWidget_, &GenerateGraphWidget::graphGenerated,
        this, &MainWindow::onGeneratedGraph);

    connect(controls_, &ControlPanelWidget::loadGraphRequested,
        this, &MainWindow::onLoadGraphClicked);

    connect(controls_, &ControlPanelWidget::runAlgorithmRequested,
        this, &MainWindow::onRunClicked);

    connect(controls_, &ControlPanelWidget::algorithmChanged,
        this, &MainWindow::onAlgorithmChanged);

    connect(controls_, &ControlPanelWidget::parameterChanged,
        this, &MainWindow::onParameterChanged);

    connect(graphWidget_, &GraphWidget::edgeSelected,
        this, &MainWindow::onEdgeSelected);
}

void MainWindow::applyStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: #070b18;
        }

        QWidget {
            color: #e8f1ff;
            font-family: "Segoe UI";
            font-size: 13px;
        }

        #Panel, #SidebarWidget {
            background-color: #0d1630;
            border: 1px solid #27446f;
            border-radius: 18px;
        }

        #SidebarWidget {
            background-color: qlineargradient(
                x1:0,y1:0,x2:1,y2:1,
                stop:0 #09142c,
                stop:1 #291045
            );
        }

        #SidebarTitle {
            color: #ffffff;
            font-size: 24px;
            font-weight: 800;
        }

        #SidebarItem {
            color: #dbeafe;
            background-color: rgba(255,255,255,0.06);
            border-radius: 10px;
            padding-left: 10px;
        }

        #SidebarItem:hover {
            background-color: rgba(125, 211, 252, 0.18);
        }

        #SidebarFooter {
            color: #89a7c7;
            font-size: 11px;
        }

        #PanelTitle {
            color: #7dd3fc;
            font-size: 17px;
            font-weight: 800;
        }

        #PanelContent {
            color: #d9e7ff;
        }

        #LogBox, QTextEdit {
            background-color: #060b17;
            border: 1px solid #233b61;
            border-radius: 10px;
            color: #cfe4ff;
        }

        QComboBox, QDoubleSpinBox, QSpinBox {
            background-color: #111c36;
            border: 1px solid #2d4d7d;
            border-radius: 9px;
            padding: 7px 10px;
            min-height: 28px;
            color: #ffffff;
        }

        QPushButton {
            background-color: #13213f;
            border: 1px solid #335b91;
            border-radius: 10px;
            padding: 9px 16px;
            color: #ffffff;
            font-weight: 600;
        }

        QPushButton:hover {
            background-color: #1d3a6a;
            border-color: #64b5ff;
        }

        QPushButton:pressed {
            background-color: #0f2a50;
        }
    )");
}

QString MainWindow::defaultDataPath() const
{
    QDir dir(QCoreApplication::applicationDirPath());

    for (int i = 0; i < 5; ++i) {
        if (dir.exists("data")) {
            dir.cd("data");
            return dir.absolutePath();
        }

        if (!dir.cdUp())
            break;
    }

    return QDir::homePath();
}

std::vector<VisualEdge> MainWindow::loadVisualEdgesFromCSV(const QString& fileName) const
{
    std::ifstream file(fileName.toStdString());

    if (!file.is_open()) {
        throw std::runtime_error("Nu pot deschide fisierul CSV pentru vizualizare.");
    }

    std::string line;
    std::getline(file, line);

    std::vector<VisualEdge> edges;
    int lineNumber = 1;

    while (std::getline(file, line)) {
        ++lineNumber;

        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> values;

        while (std::getline(ss, token, ',')) {
            values.push_back(token);
        }

        if (values.size() < 6) {
            throw std::runtime_error(
                "Linie CSV invalida la linia " + std::to_string(lineNumber) +
                ". Format asteptat: from,to,capacity,max_capacity,isNew,rate"
            );
        }

        VisualEdge edge;
        edge.from = std::stoi(values[0]);
        edge.to = std::stoi(values[1]);
        edge.capacity = std::stod(values[2]);
        edge.maxCapacity = std::stod(values[3]);
        edge.isNew = (std::stoi(values[4]) != 0);
        edge.rate = std::stod(values[5]);

        edges.push_back(edge);
    }

    return edges;
}

void MainWindow::loadGraphFromFile(const QString& fileName)
{
    lastLoadedGraphFileName_ = fileName;

    currentGraph_ = loadGraphFromCSV(fileName.toStdString(), CostModel::ArticleGeneral);
    visualEdges_ = loadVisualEdgesFromCSV(fileName);

    resetResultState();
    refreshViewState();

    graphWidget_->setEdges(visualEdges_);

    resultWidget_->appendLog("Loaded Graph from: " + fileName);
    resultWidget_->appendLog("Graph loaded successfully.");
    logSeparator();
}

void MainWindow::resetResultState()
{
    viewState_.hasResult = false;
    viewState_.resultZ = 0.0;
    viewState_.finalCost = 0.0;
    viewState_.path.clear();

    if (graphWidget_)
        graphWidget_->clearSelection();

    if (edgeDetails_)
        edgeDetails_->clearEdge();

    if (resultWidget_)
        resultWidget_->clearResult();
}

void MainWindow::refreshViewState()
{
    if (!controls_ || !graphWidget_ || !edgeDetails_)
        return;

    viewState_.algorithm = controls_->currentAlgorithm();
    viewState_.inputValue = controls_->parameterValue();

    graphWidget_->setViewState(viewState_);
    edgeDetails_->setContext(viewState_);
}

void MainWindow::logSeparator()
{
    if (resultWidget_)
        resultWidget_->appendLog("------------------------------");
}

QString MainWindow::formatPath(const std::vector<int>& path) const
{
    if (path.empty())
        return "(fara drum)";

    QString text;

    for (size_t i = 0; i < path.size(); ++i) {
        text += QString::number(path[i]);

        if (i + 1 < path.size())
            text += " -> ";
    }

    return text;
}

void MainWindow::onLoadGraphClicked()
{
    static QString lastPath;
    const QString startPath = lastPath.isEmpty() ? defaultDataPath() : lastPath;

    const QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Graph",
        startPath,
        "CSV Files (*.csv);;All Files (*.*)"
    );

    if (fileName.isEmpty())
        return;

    try {
        lastPath = QFileInfo(fileName).absolutePath();
        loadGraphFromFile(fileName);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::onGeneratedGraph(const QString& filePath)
{
    try {
        stackedWidget_->setCurrentWidget(loadRunPage_);
        loadGraphFromFile(filePath);
        resultWidget_->appendLog("Generated CSV loaded automatically.");
        logSeparator();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Generation error", e.what());
    }
}

void MainWindow::onRunClicked()
{
    if (!currentGraph_) {
        QMessageBox::warning(this, "Warning", "Please load a graph first.");
        return;
    }

    resetResultState();
    refreshViewState();

    const QString selectedAlgo = controls_->currentAlgorithm();
    const double inputValue = controls_->parameterValue();
    const QString parameterName = (selectedAlgo == "AMCMCPEP") ? "z" : "W";

    resultWidget_->appendLog(
        QString("Running %1 with %2=%3")
        .arg(selectedAlgo)
        .arg(parameterName)
        .arg(inputValue)
    );

    try {
        if (selectedAlgo == "AMCMCPEP") {
            const double z = inputValue;

            AMCMCPEP solver;
            const auto algResult = solver.solve(*currentGraph_, z);

            viewState_.hasResult = algResult.isFeasible;
            viewState_.resultZ = z;
            viewState_.finalCost = algResult.totalCost;
            viewState_.path = algResult.path;

            resultWidget_->appendLog(QString("Desired z: %1").arg(z));
            resultWidget_->appendLog(QString("Feasible: %1").arg(algResult.isFeasible ? "Yes" : "No"));
            resultWidget_->appendLog(QString("Total Cost: %1").arg(algResult.totalCost));
            resultWidget_->appendLog("Path: " + formatPath(algResult.path));

            resultWidget_->setResult(
                selectedAlgo,
                algResult.path,
                z,
                algResult.totalCost,
                algResult.isFeasible
            );
        }
        else if (selectedAlgo == "AMCPEBCP") {
            const double W = inputValue;

            AMCPEBCP solver;
            const auto algResult = solver.solve(*currentGraph_, W);

            const bool feasible = !algResult.path.empty();

            viewState_.hasResult = feasible;
            viewState_.resultZ = algResult.maxCapacity;
            viewState_.finalCost = algResult.finalCost;
            viewState_.path = algResult.path;

            resultWidget_->appendLog(QString("Budget W: %1").arg(W));
            resultWidget_->appendLog(QString("Best z*: %1").arg(algResult.maxCapacity));
            resultWidget_->appendLog(QString("Final Cost: %1").arg(algResult.finalCost));
            resultWidget_->appendLog("Path: " + formatPath(algResult.path));

            resultWidget_->setResult(
                selectedAlgo,
                algResult.path,
                algResult.maxCapacity,
                algResult.finalCost,
                feasible
            );
        }
        else if (selectedAlgo == "Parametric Search") {
            const double W = inputValue;

            if (lastLoadedGraphFileName_.isEmpty()) {
                throw std::runtime_error(
                    "Nu exista calea fisierului CSV incarcat. Reincarca graful si ruleaza din nou."
                );
            }

            Graph linearGraph = loadGraphFromCSV(
                lastLoadedGraphFileName_.toStdString(),
                CostModel::Linear
            );

            ParametricSearch solver;
            const auto algResult = solver.solve(linearGraph, W);

            const bool feasible = !algResult.path.empty();

            viewState_.hasResult = feasible;
            viewState_.resultZ = algResult.maxCapacity;
            viewState_.finalCost = algResult.finalCost;
            viewState_.path = algResult.path;

            resultWidget_->appendLog(QString("Budget W: %1").arg(W));
            resultWidget_->appendLog(QString("Best z*: %1").arg(algResult.maxCapacity));
            resultWidget_->appendLog(QString("Final Cost: %1").arg(algResult.finalCost));
            resultWidget_->appendLog("Path: " + formatPath(algResult.path));

            resultWidget_->setResult(
                selectedAlgo,
                algResult.path,
                algResult.maxCapacity,
                algResult.finalCost,
                feasible
            );
        }

        refreshViewState();
        logSeparator();
    }
    catch (const std::exception& e) {
        resultWidget_->appendLog(QString("Error executing algorithm: %1").arg(e.what()));
        logSeparator();
    }
}

void MainWindow::onAlgorithmChanged(const QString& algorithm)
{
    Q_UNUSED(algorithm);

    resetResultState();
    refreshViewState();
}

void MainWindow::onParameterChanged(double value)
{
    Q_UNUSED(value);

    resetResultState();
    refreshViewState();
}

void MainWindow::onEdgeSelected(const VisualEdge& edge)
{
    if (edgeDetails_)
        edgeDetails_->setEdge(edge);
}