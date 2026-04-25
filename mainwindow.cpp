#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GraphLoader.h"
#include <QFileDialog>
#include <QMessageBox>

// Include your algorithm headers here
#include "backend/algorithms/AMCMCPEP.h"
#include "backend/algorithms/AMCPEBCP.h"
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
    
    // Parameter input: z for Algorithm 1, W for Algorithms 3 and 4
    zSpinBox = new QDoubleSpinBox(this);
    zSpinBox->setRange(0, 10000);
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
    
    // Connections
    auto updateParameterPrefix = [this]() {
        const QString selectedAlgo = algoComboBox->currentText();

        if (selectedAlgo == "AMCMCPEP") {
            zSpinBox->setPrefix("z = ");
            zSpinBox->setToolTip("Desired MCP capacity z for Algorithm 1");
        } else {
            zSpinBox->setPrefix("W = ");
            zSpinBox->setToolTip("Expansion budget W for Algorithm 3 / Algorithm 4");
        }
    };

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
            currentGraph = loadGraphFromCSV(fileName.toStdString());
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
    double inputValue = zSpinBox->value();
    const QString parameterName = (selectedAlgo == "AMCMCPEP") ? "z" : "W";
    
    outputText->append(QString("--- Running %1 with %2=%3 ---")
        .arg(selectedAlgo)
        .arg(parameterName)
        .arg(inputValue));
    
    try {
        if (selectedAlgo == "AMCMCPEP") {
            AMCMCPEP solver;
            auto result = solver.solve(*currentGraph, inputValue);

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
            AMCPEBCP solver;
            auto result = solver.solve(*currentGraph, inputValue);

            outputText->append(QString("Budget W: %1").arg(inputValue));
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
        } else if (selectedAlgo == "Parametric Search") {
            outputText->append(QString("Budget W: %1").arg(inputValue));
            outputText->append("Parametric Search is not connected yet.");
        }
    }
    catch(const std::exception& e) {
        outputText->append(QString("Error executing algorithm: %1").arg(e.what()));
    }
    
    outputText->append("-------------------");
    outputText->ensureCursorVisible();
}
