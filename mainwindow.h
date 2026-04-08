#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRunClicked();
    void onLoadGraphClicked();

private:
    Ui::MainWindow *ui;
    
    QComboBox *algoComboBox;
    QDoubleSpinBox *zSpinBox;
    QPushButton *runButton;
    QPushButton *loadGraphButton;
    QTextEdit *outputText;
    
    // Placeholder for graph - include your graph headers here
    // Graph currentGraph; 
};

#endif // MAINWINDOW_H
