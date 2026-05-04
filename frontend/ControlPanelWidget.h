#pragma once

#include <QWidget>
#include <QString>

class QComboBox;
class QDoubleSpinBox;
class QPushButton;

class ControlPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanelWidget(QWidget* parent = nullptr);

    QString currentAlgorithm() const;
    double parameterValue() const;
    void setParameterPrefixForAlgorithm(const QString& algorithm);

signals:
    void loadGraphRequested();
    void runAlgorithmRequested();
    void algorithmChanged(const QString& algorithm);
    void parameterChanged(double value);

private:
    QComboBox* algorithmCombo_ = nullptr;
    QDoubleSpinBox* parameterSpin_ = nullptr;
    QPushButton* loadButton_ = nullptr;
    QPushButton* runButton_ = nullptr;
};
