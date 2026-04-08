#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

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
    QString fileName = QFileDialog::getOpenFileName(this, "Open Graph File", "", "All Files (*.*)");
    if (!fileName.isEmpty()) {
        try {
            // Load graph logic
            // currentGraph = Graph::loadFromFile(fileName.toStdString());
            outputText->append("Loaded Graph from: " + fileName);
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
}

void MainWindow::onRunClicked()
{
    // if (!currentGraph.isLoaded()) {
    //     QMessageBox::warning(this, "Warning", "Please load a graph first!");
    //     return;
    // }

    QString selectedAlgo = algoComboBox->currentText();
    double zValue = zSpinBox->value();
    
    outputText->append(QString("--- Running %1 with Z=%2 ---").arg(selectedAlgo).arg(zValue));
    
    try {
        if (selectedAlgo == "AMCMCPEP") {
            // AMCMCPEP solver;
            // auto result = solver.solve(currentGraph, zValue);
            // outputText->append(QString("Feasible: %1 | Total Cost: %2").arg(result.isFeasible ? "Yes" : "No").arg(result.totalCost));
            outputText->append("AMCMCPEP finished (placeholder output).");
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
