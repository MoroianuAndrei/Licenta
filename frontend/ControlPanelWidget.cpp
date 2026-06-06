#include "ControlPanelWidget.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ControlPanelWidget::ControlPanelWidget(QWidget* parent)
    : QWidget(parent)
{
    algorithmCombo_ = new QComboBox(this);
    algorithmCombo_->addItem("AMCMCPEP");
    algorithmCombo_->addItem("AMCPEBCP");
    algorithmCombo_->addItem("Parametric Search");

    parameterSpin_ = new QDoubleSpinBox(this);
    parameterSpin_->setRange(0.0, 1000000.0);
    parameterSpin_->setDecimals(2);
    parameterSpin_->setValue(20.0);
    parameterSpin_->setMinimumWidth(170);
    parameterSpin_->setPrefix("z = ");

    loadButton_ = new QPushButton("Import date (CSV)", this);
    runButton_ = new QPushButton("Ruleaza algoritm", this);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(12);
    layout->addWidget(new QLabel("Algoritm:", this));
    layout->addWidget(algorithmCombo_, 1);
    layout->addWidget(parameterSpin_);
    layout->addWidget(loadButton_);
    layout->addWidget(runButton_);

    connect(loadButton_, &QPushButton::clicked, this, &ControlPanelWidget::loadGraphRequested);
    connect(runButton_, &QPushButton::clicked, this, &ControlPanelWidget::runAlgorithmRequested);
    connect(algorithmCombo_, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        setParameterPrefixForAlgorithm(text);
        emit algorithmChanged(text);
    });
    connect(parameterSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ControlPanelWidget::parameterChanged);
}

QString ControlPanelWidget::currentAlgorithm() const
{
    return algorithmCombo_->currentText();
}

double ControlPanelWidget::parameterValue() const
{
    return parameterSpin_->value();
}

void ControlPanelWidget::setParameterPrefixForAlgorithm(const QString& algorithm)
{
    if (algorithm == "AMCMCPEP") {
        parameterSpin_->setPrefix("z = ");
        parameterSpin_->setToolTip("Capacitatea tinta z pentru MCMCPEP");
    } else {
        parameterSpin_->setPrefix("W = ");
        parameterSpin_->setToolTip("Bugetul W pentru algoritmii sub constrangere de buget");
    }
}

