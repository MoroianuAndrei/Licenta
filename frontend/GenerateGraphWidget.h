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

    QSpinBox* nodesSpinBox = nullptr;
    QSpinBox* pathsSpinBox = nullptr;
    QSpinBox* cyclesSpinBox = nullptr;
    QDoubleSpinBox* probabilitySpinBox = nullptr;
    QSpinBox* seedSpinBox = nullptr;
    QPushButton* generateButton = nullptr;
    QLabel* statusLabel = nullptr;
};
