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
    algorithmCombo_->addItem("MCP");
    algorithmCombo_->addItem("AMCMCPEP");
    algorithmCombo_->addItem("AMCPEBCP");
    algorithmCombo_->addItem("Parametric Search");

    parameterSpin_ = new QDoubleSpinBox(this);
    parameterSpin_->setRange(0.0, 1000000.0);
    parameterSpin_->setDecimals(2);
    parameterSpin_->setValue(20.0);
    parameterSpin_->setMinimumWidth(170);
    parameterSpin_->setPrefix("z = ");

    sourceButton_ = new QPushButton("s", this);
    sourceButton_->setCheckable(true);
    sourceButton_->setMinimumWidth(75);
    sourceButton_->setObjectName("SourceButton");
    sourceButton_->setToolTip("Apasa, apoi click pe un nod pentru a-l seta ca sursa");

    targetButton_ = new QPushButton("t", this);
    targetButton_->setCheckable(true);
    targetButton_->setMinimumWidth(75);
    targetButton_->setObjectName("TargetButton");
    targetButton_->setToolTip("Apasa, apoi click pe un nod pentru a-l seta ca destinatie");

    loadButton_ = new QPushButton("Import date (CSV)", this);
    runButton_ = new QPushButton("Ruleaza algoritm", this);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(12);
    layout->addWidget(new QLabel("Algoritm:", this));
    layout->addWidget(algorithmCombo_, 1);
    layout->addWidget(parameterSpin_);
    layout->addWidget(sourceButton_);
    layout->addWidget(targetButton_);
    layout->addWidget(loadButton_);
    layout->addWidget(runButton_);

    connect(loadButton_, &QPushButton::clicked, this, &ControlPanelWidget::loadGraphRequested);
    connect(runButton_, &QPushButton::clicked, this, &ControlPanelWidget::runAlgorithmRequested);
    connect(algorithmCombo_, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        setParameterPrefixForAlgorithm(text);
        emit algorithmChanged(text);
    });
    connect(parameterSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged),
        this, &ControlPanelWidget::parameterChanged);

    connect(sourceButton_, &QPushButton::toggled, this, [this](bool checked) {
        if (checked)
        {
            targetButton_->blockSignals(true);
            targetButton_->setChecked(false);
            targetButton_->blockSignals(false);
            emit selectionModeChanged(1);
        }
        else
        {
            emit selectionModeChanged(0);
        }
    });

    connect(targetButton_, &QPushButton::toggled, this, [this](bool checked) {
        if (checked)
        {
            sourceButton_->blockSignals(true);
            sourceButton_->setChecked(false);
            sourceButton_->blockSignals(false);
            emit selectionModeChanged(2);
        }
        else
        {
            emit selectionModeChanged(0);
        }
    });

    setParameterPrefixForAlgorithm(algorithmCombo_->currentText());
}

QString ControlPanelWidget::currentAlgorithm() const
{
    return algorithmCombo_->currentText();
}

double ControlPanelWidget::parameterValue() const
{
    return parameterSpin_->value();
}

int ControlPanelWidget::sourceNodeValue() const
{
    return currentSource_;
}

int ControlPanelWidget::targetNodeValue() const
{
    return currentTarget_;
}

void ControlPanelWidget::setSourceTarget(int s, int t)
{
    currentSource_ = s;
    currentTarget_ = t;
    sourceButton_->setText(QString("s: %1").arg(s));
    targetButton_->setText(QString("t: %1").arg(t));
}

void ControlPanelWidget::deactivateSelectionMode()
{
    sourceButton_->blockSignals(true);
    targetButton_->blockSignals(true);
    sourceButton_->setChecked(false);
    targetButton_->setChecked(false);
    sourceButton_->blockSignals(false);
    targetButton_->blockSignals(false);
}

void ControlPanelWidget::setParameterPrefixForAlgorithm(const QString& algorithm)
{
    if (algorithm == "MCP")
    {
        parameterSpin_->setVisible(false);
    }
    else if (algorithm == "AMCMCPEP")
    {
        parameterSpin_->setVisible(true);
        parameterSpin_->setPrefix("z = ");
        parameterSpin_->setToolTip("Capacitatea tinta z pentru MCMCPEP");
    }
    else
    {
        parameterSpin_->setVisible(true);
        parameterSpin_->setPrefix("W = ");
        parameterSpin_->setToolTip("Bugetul W pentru algoritmii sub constrangere de buget");
    }
}
