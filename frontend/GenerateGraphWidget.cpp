#include "GenerateGraphWidget.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

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

    nodesSpinBox = new QSpinBox(this);
    nodesSpinBox->setRange(2, 10000);
    nodesSpinBox->setValue(100);

    pathsSpinBox = new QSpinBox(this);
    pathsSpinBox->setRange(1, 10000);
    pathsSpinBox->setValue(20);

    cyclesSpinBox = new QSpinBox(this);
    cyclesSpinBox->setRange(0, 10000);
    cyclesSpinBox->setValue(10);

    probabilitySpinBox = new QDoubleSpinBox(this);
    probabilitySpinBox->setRange(0.0, 1.0);
    probabilitySpinBox->setDecimals(3);
    probabilitySpinBox->setSingleStep(0.01);
    probabilitySpinBox->setValue(0.05);

    seedSpinBox = new QSpinBox(this);
    seedSpinBox->setRange(0, 999999999);
    seedSpinBox->setValue(123);

    generateButton = new QPushButton("Generate CSV", this);
    statusLabel = new QLabel("No file generated yet.", this);

    const QString inputStyle =
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
        auto* label = new QLabel(labelText, this);
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

QString GenerateGraphWidget::defaultDataPath() const
{
    QDir dir(QCoreApplication::applicationDirPath());

    for (int i = 0; i < 5; ++i) {
        if (dir.exists("data")) {
            dir.cd("data");
            return dir.absolutePath();
        }

        if (!dir.cdUp()) {
            break;
        }
    }

    return QDir::homePath();
}

void GenerateGraphWidget::onGenerateClicked()
{
    const QString suggestedFileName = QString("random_graph_%1.csv").arg(nodesSpinBox->value());
    const QString suggestedPath = QDir(defaultDataPath()).filePath(suggestedFileName);

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save generated graph",
        suggestedPath,
        "CSV Files (*.csv);;All Files (*.*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    if (!filePath.endsWith(".csv", Qt::CaseInsensitive)) {
        filePath += ".csv";
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
