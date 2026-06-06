#pragma once

#include <QWidget>

class QPushButton;
class QTextEdit;
class QLabel;

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TestWidget(QWidget* parent = nullptr);

private slots:
    void onRunClicked();

private:
    void buildUi();
    void applyStyle();
    void displayOutput(const QString& text);

    QPushButton* runButton_     = nullptr;
    QTextEdit*   logBox_        = nullptr;
    QLabel*      summaryLabel_  = nullptr;
};

