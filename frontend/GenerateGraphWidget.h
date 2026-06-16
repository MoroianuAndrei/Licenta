#pragma once

#include <QWidget>

class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QLabel;

class GenerateGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GenerateGraphWidget(QWidget* parent = nullptr);

signals:
    void graphGenerated(const QString& filePath);

private slots:
    void onGenerateClicked();

private:
    QString defaultDataPath() const;

    QSpinBox* nodesSpinBox_ = nullptr;
    QSpinBox* sourceSpinBox_ = nullptr;
    QSpinBox* targetSpinBox_ = nullptr;
    QSpinBox* pathsSpinBox_ = nullptr;
    QSpinBox* cyclesSpinBox_ = nullptr;
    QDoubleSpinBox* probabilitySpinBox_ = nullptr;
    QSpinBox* minCapacitySpinBox_ = nullptr;
    QSpinBox* maxCapacitySpinBox_ = nullptr;
    QSpinBox* maxCapacityLimitSpinBox_ = nullptr;
    QSpinBox* seedSpinBox_ = nullptr;
    QPushButton* generateButton_ = nullptr;
    QLabel* statusLabel_ = nullptr;
};

