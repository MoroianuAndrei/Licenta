#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QTableWidget>
#include <QProgressBar>
#include <QThread>
#include <QLabel>
#include <vector>
#include "backend/testing/PerformanceTest.h"

// ----------------------------------------------------------------
// Worker thread — ruleaza benchmark-ul fara sa blocheze UI-ul
// ----------------------------------------------------------------
class BenchmarkWorker : public QObject
{
    Q_OBJECT

public:
    BenchmarkWorker(std::vector<int> sizes, double W, unsigned int seed)
        : sizes_(std::move(sizes)), W_(W), seed_(seed) {
    }

public slots:
    void run();

signals:
    void progressUpdate(int current, int total, int nodes);
    void entryReady(BenchmarkEntry entry);
    void finished(std::vector<BenchmarkEntry> results);
    void errorOccurred(QString message);

private:
    std::vector<int> sizes_;
    double           W_;
    unsigned int     seed_;
};

// ----------------------------------------------------------------
// BenchmarkWidget — panoul principal din UI
// ----------------------------------------------------------------
class BenchmarkWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BenchmarkWidget(QWidget* parent = nullptr);

signals:
    void benchmarkFinished(std::vector<BenchmarkEntry> results);

private slots:
    void onRunClicked();
    void onSaveClicked();
    void onProgressUpdate(int current, int total, int nodes);
    void onEntryReady(BenchmarkEntry entry);
    void onFinished(std::vector<BenchmarkEntry> results);
    void onError(QString message);

private:
    void buildUi();
    void applyStyle();
    void addTableRow(const BenchmarkEntry& e, int row);
    void setRunning(bool running);

    // Input
    QLineEdit* nodesInput_;
    QDoubleSpinBox* budgetInput_;

    // Controls
    QPushButton* runButton_;
    QPushButton* saveButton_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;

    // Results
    QTableWidget* resultsTable_;
    QTextEdit* logBox_;

    // Data
    std::vector<BenchmarkEntry> results_;
    QThread* workerThread_ = nullptr;
};