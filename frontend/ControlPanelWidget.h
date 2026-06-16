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
    int sourceNodeValue() const;
    int targetNodeValue() const;

    void setParameterPrefixForAlgorithm(const QString& algorithm);
    void setSourceTarget(int s, int t);
    void deactivateSelectionMode();

signals:
    void loadGraphRequested();
    void runAlgorithmRequested();
    void algorithmChanged(const QString& algorithm);
    void parameterChanged(double value);
    void selectionModeChanged(int mode);

private:
    QComboBox* algorithmCombo_ = nullptr;
    QDoubleSpinBox* parameterSpin_ = nullptr;
    QPushButton* sourceButton_ = nullptr;
    QPushButton* targetButton_ = nullptr;
    QPushButton* loadButton_ = nullptr;
    QPushButton* runButton_ = nullptr;

    int currentSource_ = 1;
    int currentTarget_ = 0;
};
