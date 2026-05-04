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
    QSpinBox* nodesSpinBox;
    QSpinBox* pathsSpinBox;
    QSpinBox* cyclesSpinBox;
    QDoubleSpinBox* probabilitySpinBox;
    QSpinBox* seedSpinBox;
    QPushButton* generateButton;
    QLabel* statusLabel;
};