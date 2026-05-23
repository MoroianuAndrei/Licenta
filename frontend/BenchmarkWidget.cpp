#include "BenchmarkWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>

#include <sstream>

// ----------------------------------------------------------------
// BenchmarkWorker
// ----------------------------------------------------------------
void BenchmarkWorker::run()
{
    PerformanceTest test;
    std::vector<BenchmarkEntry> results;

    int total = (int)sizes_.size();

    for (int i = 0; i < total; ++i)
    {
        int n = sizes_[i];
        emit progressUpdate(i + 1, total, n);

        try
        {
            BenchmarkEntry entry = test.runSingle(n, W_, seed_);
            results.push_back(entry);
            emit entryReady(entry);
        }
        catch (const std::exception& ex)
        {
            emit errorOccurred(
                QString("Eroare la n=%1: %2").arg(n).arg(ex.what())
            );
        }
    }

    emit finished(results);
}

// ----------------------------------------------------------------
// BenchmarkWidget
// ----------------------------------------------------------------
BenchmarkWidget::BenchmarkWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    applyStyle();
}

void BenchmarkWidget::buildUi()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(12);

    // --- Titlu ---
    auto* title = new QLabel("Benchmark: AMCPEBCP vs Parametric Search");
    title->setObjectName("PanelTitle");
    rootLayout->addWidget(title);

    // --- Parametri ---
    auto* paramGroup = new QGroupBox("Parametri benchmark");
    paramGroup->setObjectName("Panel");
    auto* paramLayout = new QGridLayout(paramGroup);
    paramLayout->setSpacing(10);

    paramLayout->addWidget(new QLabel("Dimensiuni grafuri (noduri, separate prin virgulă):"), 0, 0);
    nodesInput_ = new QLineEdit("10, 20, 50, 100, 200");
    nodesInput_->setPlaceholderText("ex: 10, 50, 100, 500, 1000");
    paramLayout->addWidget(nodesInput_, 0, 1);

    paramLayout->addWidget(new QLabel("Buget W:"), 1, 0);
    budgetInput_ = new QDoubleSpinBox();
    budgetInput_->setRange(1.0, 1e9);
    budgetInput_->setValue(1000.0);
    budgetInput_->setDecimals(2);
    paramLayout->addWidget(budgetInput_, 1, 1);

    rootLayout->addWidget(paramGroup);

    // --- Butoane + progress ---
    auto* ctrlLayout = new QHBoxLayout();

    runButton_ = new QPushButton("Rulează Benchmark");
    runButton_->setObjectName("RunButton");
    ctrlLayout->addWidget(runButton_);

    saveButton_ = new QPushButton("Salvează CSV");
    saveButton_->setEnabled(false);
    ctrlLayout->addWidget(saveButton_);

    rootLayout->addLayout(ctrlLayout);

    progressBar_ = new QProgressBar();
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setVisible(false);
    rootLayout->addWidget(progressBar_);

    statusLabel_ = new QLabel("");
    statusLabel_->setObjectName("PanelContent");
    rootLayout->addWidget(statusLabel_);

    // --- Tabel rezultate ---
    auto* tableLabel = new QLabel("Rezultate:");
    tableLabel->setObjectName("PanelTitle");
    rootLayout->addWidget(tableLabel);

    resultsTable_ = new QTableWidget(0, 10);
    resultsTable_->setHorizontalHeaderLabels({
        "Noduri", "Arce", "W",
        "AMCPEBCP (ms)", "Apeluri AMCPEBCP", "z* AMCPEBCP",
        "Param. Search (ms)", "Apeluri Param.", "z* Param.",
        "Δz*"
        });
    resultsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    resultsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTable_->setAlternatingRowColors(true);
    rootLayout->addWidget(resultsTable_, 1);

    // --- Log ---
    auto* logLabel = new QLabel("Log:");
    logLabel->setObjectName("PanelContent");
    rootLayout->addWidget(logLabel);

    logBox_ = new QTextEdit();
    logBox_->setObjectName("LogBox");
    logBox_->setReadOnly(true);
    logBox_->setMaximumHeight(120);
    rootLayout->addWidget(logBox_);

    // --- Conexiuni ---
    connect(runButton_, &QPushButton::clicked, this, &BenchmarkWidget::onRunClicked);
    connect(saveButton_, &QPushButton::clicked, this, &BenchmarkWidget::onSaveClicked);
}

void BenchmarkWidget::applyStyle()
{
    setStyleSheet(R"(
        QGroupBox {
            border: 1px solid #27446f;
            border-radius: 10px;
            margin-top: 8px;
            padding: 8px;
            color: #e8f1ff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            color: #7dd3fc;
        }
        QTableWidget {
            background-color: #060b17;
            border: 1px solid #233b61;
            border-radius: 8px;
            color: #cfe4ff;
            gridline-color: #1a2e4a;
        }
        QTableWidget::item:alternate {
            background-color: #0a1628;
        }
        QHeaderView::section {
            background-color: #0d1630;
            color: #7dd3fc;
            border: 1px solid #27446f;
            padding: 4px;
            font-weight: bold;
        }
        #RunButton {
            background-color: #1a4a8a;
            border: 1px solid #3a7bd5;
            border-radius: 10px;
            padding: 10px 20px;
            color: #ffffff;
            font-weight: 700;
            font-size: 13px;
        }
        #RunButton:hover {
            background-color: #2560b0;
        }
        #RunButton:disabled {
            background-color: #0d1630;
            color: #555;
        }
    )");
}

void BenchmarkWidget::addTableRow(const BenchmarkEntry& e, int row)
{
    resultsTable_->insertRow(row);

    auto addItem = [&](int col, const QString& text, bool highlight = false) {
        auto* item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        if (highlight)
            item->setForeground(QColor("#2ecc71"));
        resultsTable_->setItem(row, col, item);
        };

    addItem(0, QString::number(e.nodes));
    addItem(1, QString::number(e.edges));
    addItem(2, QString::number(e.W, 'f', 1));

    // AMCPEBCP
    addItem(3, QString::number(e.amcpebcp_time_ms, 'f', 2));
    addItem(4, QString::number(e.amcpebcp_oracle_calls));
    addItem(5, QString::number(e.amcpebcp_result_z, 'f', 4));

    // Parametric Search — evidentiaza daca e mai rapid
    bool paramFaster = e.parametric_time_ms < e.amcpebcp_time_ms;
    addItem(6, QString::number(e.parametric_time_ms, 'f', 2), paramFaster);
    addItem(7, QString::number(e.parametric_oracle_calls), paramFaster);
    addItem(8, QString::number(e.parametric_result_z, 'f', 4));

    // Diferenta z*
    bool correct = e.z_difference < 0.01;
    auto* diffItem = new QTableWidgetItem(
        QString::number(e.z_difference, 'f', 6)
    );
    diffItem->setTextAlignment(Qt::AlignCenter);
    diffItem->setForeground(correct ? QColor("#2ecc71") : QColor("#e74c3c"));
    resultsTable_->setItem(row, 9, diffItem);
}

void BenchmarkWidget::setRunning(bool running)
{
    runButton_->setEnabled(!running);
    nodesInput_->setEnabled(!running);
    budgetInput_->setEnabled(!running);
    progressBar_->setVisible(running);
    saveButton_->setEnabled(!running && !results_.empty());
}

void BenchmarkWidget::onRunClicked()
{
    // Parseaza dimensiunile
    std::vector<int> sizes;
    QString input = nodesInput_->text();
    QStringList parts = input.split(',', Qt::SkipEmptyParts);

    for (const QString& p : parts)
    {
        bool ok = false;
        int n = p.trimmed().toInt(&ok);
        if (ok && n > 1)
            sizes.push_back(n);
        else
        {
            QMessageBox::warning(this, "Input invalid",
                QString("Valoare invalida: '%1'. Foloseste numere intregi > 1.").arg(p.trimmed()));
            return;
        }
    }

    if (sizes.empty())
    {
        QMessageBox::warning(this, "Input invalid", "Introdu cel putin o dimensiune.");
        return;
    }

    // Reset UI
    results_.clear();
    resultsTable_->setRowCount(0);
    logBox_->clear();
    progressBar_->setRange(0, (int)sizes.size());
    progressBar_->setValue(0);
    setRunning(true);

    double W = budgetInput_->value();

    logBox_->append(QString("Pornire benchmark: %1 dimensiuni, W=%2")
        .arg(sizes.size()).arg(W));

    // Pornire worker thread
    workerThread_ = new QThread(this);
    auto* worker = new BenchmarkWorker(sizes, W, 42);
    worker->moveToThread(workerThread_);

    connect(workerThread_, &QThread::started, worker, &BenchmarkWorker::run);
    connect(worker, &BenchmarkWorker::progressUpdate, this, &BenchmarkWidget::onProgressUpdate);
    connect(worker, &BenchmarkWorker::entryReady, this, &BenchmarkWidget::onEntryReady);
    connect(worker, &BenchmarkWorker::finished, this, &BenchmarkWidget::onFinished);
    connect(worker, &BenchmarkWorker::errorOccurred, this, &BenchmarkWidget::onError);
    connect(worker, &BenchmarkWorker::finished, workerThread_, &QThread::quit);
    connect(workerThread_, &QThread::finished, worker, &QObject::deleteLater);

    workerThread_->start();
}

void BenchmarkWidget::onProgressUpdate(int current, int total, int nodes)
{
    progressBar_->setValue(current);
    statusLabel_->setText(
        QString("Procesare %1/%2 — n=%3 noduri...").arg(current).arg(total).arg(nodes)
    );
}

void BenchmarkWidget::onEntryReady(BenchmarkEntry entry)
{
    int row = resultsTable_->rowCount();
    addTableRow(entry, row);

    logBox_->append(
        QString("n=%1 | AMCPEBCP: %2ms (z*=%3) | Parametric: %4ms (z*=%5) | Δz=%6")
        .arg(entry.nodes)
        .arg(entry.amcpebcp_time_ms, 0, 'f', 2)
        .arg(entry.amcpebcp_result_z, 0, 'f', 4)
        .arg(entry.parametric_time_ms, 0, 'f', 2)
        .arg(entry.parametric_result_z, 0, 'f', 4)
        .arg(entry.z_difference, 0, 'f', 6)
    );
}

void BenchmarkWidget::onFinished(std::vector<BenchmarkEntry> results)
{
    results_ = std::move(results);
    setRunning(false);
    statusLabel_->setText(
        QString("Benchmark finalizat! %1 teste rulate.").arg(results_.size())
    );
    logBox_->append("=== Benchmark complet! ===");
    emit benchmarkFinished(results_);
}

void BenchmarkWidget::onError(QString message)
{
    logBox_->append("EROARE: " + message);
}

void BenchmarkWidget::onSaveClicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Salvează rezultate benchmark", "benchmark_results.csv",
        "CSV Files (*.csv)"
    );

    if (fileName.isEmpty())
        return;

    PerformanceTest test;
    test.saveToCSV(results_, fileName.toStdString());

    logBox_->append("Rezultate salvate: " + fileName);
}