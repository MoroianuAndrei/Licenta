#pragma once

#include <QWidget>

class QPushButton;

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget* parent = nullptr);


    void setActiveButton(int index);

signals:
    void generateGraphClicked();
    void loadRunClicked();
    void benchmarkClicked();
    void testsClicked();

private:
    void applyButtonStyles();

    QPushButton* generateButton = nullptr;
    QPushButton* loadRunButton = nullptr;
    QPushButton* benchmarkButton = nullptr;
    QPushButton* testsButton = nullptr;

    int activeIndex_ = 1;

};

