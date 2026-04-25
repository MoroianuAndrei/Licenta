#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GraphLoader.h"

#include <QFileDialog>
#include <QMessageBox>
#include <stdexcept>

// Include your algorithm headers here
#include "backend/algorithms/AMCMCPEP.h"
#include "backend/algorithms/AMCPEBCP.h"
#include "backend/algorithms/ParametricSearch.h"

namespace
{
    QString g_lastLoadedGraphFileName;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Central Widget Setup
    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Controls Layout
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    // Algorithm Selection
    algoComboBox = new QComboBox(this);
    algoComboBox->addItem("AMCMCPEP");
    algoComboBox->addItem("AMCPEBCP");
    algoComboBox->addItem("Parametric Search");

    // Parameter input: z for Algorithm 1, W for Algorithms 3 and 4
    zSpinBox = new QDoubleSpinBox(this);
    zSpinBox->setRange(0, 1000000);
    zSpinBox->setDecimals(0);
    zSpinBox->setPrefix("z = ");

    // Buttons
    loadGraphButton = new QPushButton("Load Graph", this);
    runButton = new QPushButton("Run Algorithm", this);

    // Add to controls layout
    controlsLayout->addWidget(algoComboBox);
    controlsLayout->addWidget(zSpinBox);
    controlsLayout->addWidget(loadGraphButton);
    controlsLayout->addWidget(runButton);

    // Output Text Area
    outputText = new QTextEdit(this);
    outputText->setReadOnly(true);

    // Add to main layout
    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(outputText);

    auto updateParameterPrefix = [this]() {
        const QString selectedAlgo = algoComboBox->currentText();

        if (selectedAlgo == "AMCMCPEP") {
            zSpinBox->setPrefix("z = ");
            zSpinBox->setToolTip("Capacitatea dorita z pentru Algoritmul 1");
        } else {
            zSpinBox->setPrefix("W = ");
            zSpinBox->setToolTip("Bugetul W pentru Algoritmul 3 / Algoritmul 4");
        }
    };

    // Connections
    connect(algoComboBox, &QComboBox::currentTextChanged, this, updateParameterPrefix);
    connect(loadGraphButton, &QPushButton::clicked, this, &MainWindow::onLoadGraphClicked);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::onRunClicked);

    updateParameterPrefix();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLoadGraphClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Graph File",
        "",
        "CSV Files (*.csv);;All Files (*.*)"
    );

    if (!fileName.isEmpty()) {
        try {
            // Graful incarcat in memorie foloseste costurile generale din articol,
            // potrivite pentru Algoritmul 1 si Algoritmul 3.
            currentGraph = loadGraphFromCSV(fileName.toStdString(), CostModel::ArticleGeneral);
            g_lastLoadedGraphFileName = fileName;

            outputText->append("Loaded Graph from: " + fileName);
            outputText->append("Graph loaded successfully.");
        }
        catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
}

void MainWindow::onRunClicked()
{
    if (!currentGraph) {
        QMessageBox::warning(this, "Warning", "Please load a graph first!");
        return;
    }

    const QString selectedAlgo = algoComboBox->currentText();
    const double inputValue = zSpinBox->value();
    const QString parameterName = (selectedAlgo == "AMCMCPEP") ? "z" : "W";

    outputText->append(QString("--- Running %1 with %2=%3 ---")
        .arg(selectedAlgo)
        .arg(parameterName)
        .arg(inputValue));

    try {
        if (selectedAlgo == "AMCMCPEP") {
            const double z = inputValue;

            AMCMCPEP solver;
            auto result = solver.solve(*currentGraph, z);

            outputText->append(QString("Desired z: %1").arg(z));
            outputText->append(QString("Feasible: %1")
                .arg(result.isFeasible ? "Yes" : "No"));
            outputText->append(QString("Total Cost: %1")
                .arg(result.totalCost));

            QString pathStr = "Path: ";
            if (result.path.empty()) {
                pathStr += "(no path found)";
            } else {
                for (size_t i = 0; i < result.path.size(); ++i) {
                    pathStr += QString::number(result.path[i]);
                    if (i + 1 < result.path.size())
                        pathStr += " -> ";
                }
            }
            outputText->append(pathStr);
        }
        else if (selectedAlgo == "AMCPEBCP") {
            const double W = inputValue;

            AMCPEBCP solver;
            auto result = solver.solve(*currentGraph, W);

            outputText->append(QString("Budget W: %1").arg(W));
            outputText->append(QString("Best z*: %1").arg(result.maxCapacity));
            outputText->append(QString("Final Cost: %1").arg(result.finalCost));

            QString pathStr = "Path: ";
            if (result.path.empty()) {
                pathStr += "(no path found)";
            } else {
                for (size_t i = 0; i < result.path.size(); ++i) {
                    pathStr += QString::number(result.path[i]);
                    if (i + 1 < result.path.size())
                        pathStr += " -> ";
                }
            }
            outputText->append(pathStr);
        }
        else if (selectedAlgo == "Parametric Search") {
            const double W = inputValue;
            outputText->append(QString("Budget W: %1").arg(W));

            if (g_lastLoadedGraphFileName.isEmpty()) {
                throw std::runtime_error("Nu exista calea fisierului CSV incarcat. Reincarca graful si ruleaza din nou.");
            }

            // Algoritmul 4 functioneaza pentru cazul liniar. De aceea reconstruim
            // graful cu CostModel::Linear, folosind coloana rate din CSV:
            // b(w,z) = r_w * max(0, z - c(w)), u(w,z) = 0.
            Graph linearGraph = loadGraphFromCSV(g_lastLoadedGraphFileName.toStdString(), CostModel::Linear);

            ParametricSearch solver;
            auto result = solver.solve(linearGraph, W);

            outputText->append(QString("Best z*: %1").arg(result.maxCapacity));
            outputText->append(QString("Final Cost: %1").arg(result.finalCost));

            QString pathStr = "Path: ";
            if (result.path.empty()) {
                pathStr += "(no path found)";
            } else {
                for (size_t i = 0; i < result.path.size(); ++i) {
                    pathStr += QString::number(result.path[i]);
                    if (i + 1 < result.path.size())
                        pathStr += " -> ";
                }
            }
            outputText->append(pathStr);
        }
    }
    catch(const std::exception& e) {
        outputText->append(QString("Error executing algorithm: %1").arg(e.what()));
    }

    outputText->append("-------------------");
    outputText->ensureCursorVisible();
}
