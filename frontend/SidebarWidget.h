#pragma once

#include <QWidget>

class QPushButton;

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget* parent = nullptr);

signals:
    void generateGraphClicked();
    void loadRunClicked();

private:
    QPushButton* generateButton;
    QPushButton* loadRunButton;
};