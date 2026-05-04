#include "GenerateGraphWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include "backend/generation/RandomGraphGenerator.h"

GenerateGraphWidget::GenerateGraphWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(18);

    auto* title = new QLabel("Generate Random Graph CSV");
    title->setStyleSheet(
        "font-size: 26px;"
        "font-weight: 800;"
        "color: white;"
    );

    auto* subtitle = new QLabel(
        "Generate random directed graphs for simulations and benchmark tests."
    );
    subtitle->setStyleSheet("color: #94a3b8; font-size: 14px;");

    nodesSpinBox = new QSpinBox();
    nodesSpinBox->setRange(2, 10000);
    nodesSpinBox->setValue(100);

    pathsSpinBox = new QSpinBox();
    pathsSpinBox->setRange(1, 10000);
    pathsSpinBox->setValue(20);

    cyclesSpinBox = new QSpinBox();
    cyclesSpinBox->setRange(0, 10000);
    cyclesSpinBox->setValue(10);

    probabilitySpinBox = new QDoubleSpinBox();
    probabilitySpinBox->setRange(0.0, 1.0);
    probabilitySpinBox->setDecimals(3);
    probabilitySpinBox->setSingleStep(0.01);
    probabilitySpinBox->setValue(0.05);

    seedSpinBox = new QSpinBox();
    seedSpinBox->setRange(0, 999999999);
    seedSpinBox->setValue(123);

    generateButton = new QPushButton("Generate CSV");
    statusLabel = new QLabel("No file generated yet.");

    QString inputStyle =
        "QSpinBox, QDoubleSpinBox {"
        "background-color: #111827;"
        "color: white;"
        "border: 1px solid #334155;"
        "border-radius: 8px;"
        "padding: 8px;"
        "font-size: 14px;"
        "}";

    nodesSpinBox->setStyleSheet(inputStyle);
    pathsSpinBox->setStyleSheet(inputStyle);
    cyclesSpinBox->setStyleSheet(inputStyle);
    probabilitySpinBox->setStyleSheet(inputStyle);
    seedSpinBox->setStyleSheet(inputStyle);

    generateButton->setStyleSheet(
        "QPushButton {"
        "background-color: #2563eb;"
        "color: white;"
        "border: none;"
        "border-radius: 10px;"
        "padding: 12px;"
        "font-size: 15px;"
        "font-weight: 700;"
        "}"
        "QPushButton:hover {"
        "background-color: #1d4ed8;"
        "}"
    );

    statusLabel->setStyleSheet("color: #22c55e; font-size: 13px;");

    auto addRow = [&](const QString& labelText, QWidget* input) {
        auto* row = new QHBoxLayout();
        auto* label = new QLabel(labelText);
        label->setFixedWidth(180);
        label->setStyleSheet("color: white; font-size: 14px;");
        row->addWidget(label);
        row->addWidget(input);
        layout->addLayout(row);
        };

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(20);

    addRow("Number of nodes:", nodesSpinBox);
    addRow("Number of s-t paths:", pathsSpinBox);
    addRow("Number of cycles:", cyclesSpinBox);
    addRow("Extra arc probability:", probabilitySpinBox);
    addRow("Seed:", seedSpinBox);

    layout->addSpacing(20);
    layout->addWidget(generateButton);
    layout->addWidget(statusLabel);
    layout->addStretch();

    setStyleSheet("background-color: #0f172a;");

    connect(generateButton, &QPushButton::clicked,
        this, &GenerateGraphWidget::onGenerateClicked);
}

void GenerateGraphWidget::onGenerateClicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save generated graph",
        "random_graph.csv",
        "CSV Files (*.csv)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    try {
        RandomGraphParams params;
        params.nodes = nodesSpinBox->value();
        params.paths = pathsSpinBox->value();
        params.cycles = cyclesSpinBox->value();
        params.extraArcProbability = probabilitySpinBox->value();
        params.seed = static_cast<unsigned int>(seedSpinBox->value());

        params.minCapacity = 50;
        params.maxCapacity = 100;
        params.maxCapacityLimit = 200;
        params.minRate = 1;
        params.maxRate = 20;
        params.newArcProbability = 0.10;

        RandomGraphGenerator generator(params);
        generator.generateToCSV(filePath.toStdString());

        statusLabel->setText("Generated: " + filePath);
        emit graphGenerated(filePath);
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Generation error", e.what());
    }
}