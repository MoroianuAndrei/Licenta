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
    setStyleSheet("background-color: #0f172a;");

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(28, 20, 28, 16);
    outer->setSpacing(0);

    auto* title = new QLabel("Generate Random Graph CSV", this);
    title->setStyleSheet("font-size: 26px; font-weight: 800; color: white; background: transparent;");

    auto* subtitle = new QLabel("Generate random directed graphs for simulations and benchmark tests.", this);
    subtitle->setStyleSheet("color: #94a3b8; font-size: 14px; background: transparent;");

    nodesSpinBox_          = new QSpinBox(this);
    sourceSpinBox_         = new QSpinBox(this);
    targetSpinBox_         = new QSpinBox(this);
    pathsSpinBox_          = new QSpinBox(this);
    cyclesSpinBox_         = new QSpinBox(this);
    probabilitySpinBox_    = new QDoubleSpinBox(this);
    minCapacitySpinBox_    = new QSpinBox(this);
    maxCapacitySpinBox_    = new QSpinBox(this);
    maxCapacityLimitSpinBox_ = new QSpinBox(this);
    seedSpinBox_           = new QSpinBox(this);

    const QString spinStyle =
        "QSpinBox, QDoubleSpinBox {"
        " background-color: #111827; color: white;"
        " border: 1px solid #334155; border-radius: 8px;"
        " padding: 8px; font-size: 14px;"
        "}"
        "QSpinBox::up-button, QDoubleSpinBox::up-button,"
        "QSpinBox::down-button, QDoubleSpinBox::down-button { width: 20px; }";

    for (auto* sb : { nodesSpinBox_, sourceSpinBox_, targetSpinBox_, pathsSpinBox_,
                      cyclesSpinBox_, minCapacitySpinBox_, maxCapacitySpinBox_,
                      maxCapacityLimitSpinBox_, seedSpinBox_ })
        sb->setStyleSheet(spinStyle);
    probabilitySpinBox_->setStyleSheet(spinStyle);

    nodesSpinBox_->setRange(2, 10000);              nodesSpinBox_->setValue(20);
    sourceSpinBox_->setRange(1, 10000);             sourceSpinBox_->setValue(1);
    targetSpinBox_->setRange(1, 10000);             targetSpinBox_->setValue(20);
    pathsSpinBox_->setRange(1, 10000);              pathsSpinBox_->setValue(4);
    cyclesSpinBox_->setRange(0, 10000);             cyclesSpinBox_->setValue(2);
    probabilitySpinBox_->setRange(0.0, 1.0);        probabilitySpinBox_->setValue(0.05);
    probabilitySpinBox_->setDecimals(3);            probabilitySpinBox_->setSingleStep(0.01);
    minCapacitySpinBox_->setRange(1, 1000000000);   minCapacitySpinBox_->setValue(1);
    maxCapacitySpinBox_->setRange(2, 1000000000);   maxCapacitySpinBox_->setValue(100);
    maxCapacityLimitSpinBox_->setRange(2, 2000000000); maxCapacityLimitSpinBox_->setValue(1000);
    seedSpinBox_->setRange(0, 999999999);           seedSpinBox_->setValue(123);

    generateButton_ = new QPushButton("Generate CSV", this);
    generateButton_->setStyleSheet(
        "QPushButton {"
        " background-color: #2563eb; color: white;"
        " border: none; border-radius: 10px;"
        " padding: 12px; font-size: 15px; font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #1d4ed8; }"
        "QPushButton:pressed { background-color: #1e40af; }"
    );

    statusLabel_ = new QLabel("No file generated yet.", this);
    statusLabel_->setStyleSheet("color: #22c55e; font-size: 13px; background: transparent;");

    const QString labelStyle = "color: #cbd5e1; font-size: 14px; background: transparent;";

    auto addRow = [&](const QString& labelText, QWidget* input) {
        auto* row = new QHBoxLayout();
        row->setSpacing(16);
        auto* label = new QLabel(labelText, this);
        label->setFixedWidth(190);
        label->setStyleSheet(labelStyle);
        row->addWidget(label);
        row->addWidget(input);
        outer->addLayout(row);
    };

    outer->setSpacing(8);
    outer->addWidget(title);
    outer->addWidget(subtitle);
    outer->addSpacing(12);
    addRow("Number of nodes:",       nodesSpinBox_);
    addRow("Source node (s):",       sourceSpinBox_);
    addRow("Target node (t):",       targetSpinBox_);
    addRow("Number of s-t paths:",   pathsSpinBox_);
    addRow("Number of cycles:",      cyclesSpinBox_);
    addRow("Extra arc probability:",  probabilitySpinBox_);
    addRow("Min Capacity:",          minCapacitySpinBox_);
    addRow("Max Capacity:",          maxCapacitySpinBox_);
    addRow("Max Capacity Limit:",    maxCapacityLimitSpinBox_);
    addRow("Seed:",                  seedSpinBox_);
    outer->addStretch();
    outer->addSpacing(12);
    outer->addWidget(statusLabel_);
    outer->addSpacing(8);
    outer->addWidget(generateButton_);

    connect(nodesSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int n) {
        sourceSpinBox_->setMaximum(n);
        targetSpinBox_->setMaximum(n);
        if (targetSpinBox_->value() > n) targetSpinBox_->setValue(n);
        if (sourceSpinBox_->value() > n) sourceSpinBox_->setValue(n);
    });

    connect(generateButton_, &QPushButton::clicked, this, &GenerateGraphWidget::onGenerateClicked);
}

QString GenerateGraphWidget::defaultDataPath() const
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 5; ++i)
    {
        if (dir.exists("data")) { dir.cd("data"); return dir.absolutePath(); }
        if (!dir.cdUp()) break;
    }
    return QDir::homePath();
}

void GenerateGraphWidget::onGenerateClicked()
{
    const QString suggestedFileName = QString("random_graph_%1.csv").arg(nodesSpinBox_->value());
    const QString suggestedPath = QDir(defaultDataPath()).filePath(suggestedFileName);

    QString filePath = QFileDialog::getSaveFileName(
        this, "Save generated graph", suggestedPath,
        "CSV Files (*.csv);;All Files (*.*)"
    );

    if (filePath.isEmpty()) return;

    if (!filePath.endsWith(".csv", Qt::CaseInsensitive))
        filePath += ".csv";

    const int src = sourceSpinBox_->value();
    const int tgt = targetSpinBox_->value();
    if (src == tgt)
    {
        QMessageBox::warning(this, "Invalid nodes", "Source and target nodes must be different.");
        return;
    }

    try
    {
        RandomGraphParams params;
        params.nodes  = nodesSpinBox_->value();
        params.source = src;
        params.target = tgt;
        params.paths  = pathsSpinBox_->value();
        params.cycles = cyclesSpinBox_->value();
        params.extraArcProbability = probabilitySpinBox_->value();
        params.seed   = static_cast<unsigned int>(seedSpinBox_->value());
        params.minCapacity      = minCapacitySpinBox_->value();
        params.maxCapacity      = maxCapacitySpinBox_->value();
        params.maxCapacityLimit = maxCapacityLimitSpinBox_->value();
        params.minRate = 1;
        params.maxRate = 20;
        params.newArcProbability = 0.10;

        RandomGraphGenerator generator(params);
        generator.generateToCSV(filePath.toStdString());

        statusLabel_->setText("Generated: " + filePath);
        emit graphGenerated(filePath);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Generation error", e.what());
    }
}
