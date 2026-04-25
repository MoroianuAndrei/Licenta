#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GraphLoader.h"
#include <QFileDialog>
#include <QMessageBox>

namespace
{
    QString loadedCsvFileName;

    CostModel costModelForAlgorithm(const QString& algorithmName)
    {
        if (algorithmName == "Parametric Search")
            return CostModel::Linear;

        return CostModel::ArticleGeneral;
    }
}

// Include your algorithm headers here
#include "backend/algorithms/AMCMCPEP.h"
// #include "backend/algorithms/AMCPEBCP.h"
// #include "backend/algorithms/ParametricSearch.h"

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

    // Z Parameter input
    zSpinBox = new QDoubleSpinBox(this);
    zSpinBox->setRange(0, 10000);
    zSpinBox->setPrefix("Z = ");

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

    // Connections
    connect(loadGraphButton, &QPushButton::clicked, this, &MainWindow::onLoadGraphClicked);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::onRunClicked);
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
            loadedCsvFileName = fileName;
            currentGraph = loadGraphFromCSV(
                fileName.toStdString(),
                costModelForAlgorithm(algoComboBox->currentText())
            );

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

    QString selectedAlgo = algoComboBox->currentText();
    double zValue = zSpinBox->value();

    // Reconstruim graful inainte de rulare, deoarece Algoritmul 1/3 si Algoritmul 4
    // folosesc modele diferite pentru functiile de cost b(w,z) si u(w,z).
    if (!loadedCsvFileName.isEmpty()) {
        currentGraph = loadGraphFromCSV(
            loadedCsvFileName.toStdString(),
            costModelForAlgorithm(selectedAlgo)
        );
    }

    outputText->append(QString("--- Running %1 with Z=%2 ---").arg(selectedAlgo).arg(zValue));

    try {
        if (selectedAlgo == "AMCMCPEP") {
            AMCMCPEP solver;
            auto result = solver.solve(*currentGraph, zValue);

            outputText->append(QString("Feasible: %1")
                .arg(result.isFeasible ? "Yes" : "No"));

            outputText->append(QString("Total Cost: %1")
                .arg(result.totalCost));

            QString pathStr = "Path: ";
            for (size_t i = 0; i < result.path.size(); ++i) {
                pathStr += QString::number(result.path[i]);
                if (i + 1 < result.path.size())
                    pathStr += " -> ";
            }

            outputText->append(pathStr);
        } else if (selectedAlgo == "AMCPEBCP") {
            outputText->append("AMCPEBCP finished (placeholder output).");
        } else if (selectedAlgo == "Parametric Search") {
            outputText->append("Parametric Search finished (placeholder output).");
        }
    }
    catch(const std::exception& e) {
        outputText->append(QString("Error executing algorithm: %1").arg(e.what()));
    }

    outputText->append("-------------------");
    outputText->ensureCursorVisible();
}
