#include "BenchmarkWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QColor>
#include <QFont>

#include <sstream>


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
            BenchmarkEntry entry = test.runSingle(n, W_, maxCapLimit_, seed_);
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


    auto* title = new QLabel("Benchmark: AMCPEBCP vs Parametric Search");
    title->setObjectName("PanelTitle");
    rootLayout->addWidget(title);


    auto* desc = new QLabel(
        "Ambii algoritmi ruleaza pe acelasi graf generat random cu cost liniar.\n"
        "Alg3 face O(log(interval)) apeluri oracle [slab polinomial].\n"
        "Alg4 face O(n\xC2\xB2) apeluri oracle [puternic polinomial] — castiga cand intervalul e mare si n e mic."
    );
    desc->setObjectName("PanelContent");
    desc->setWordWrap(true);
    rootLayout->addWidget(desc);


    auto* paramGroup = new QGroupBox("Parametri benchmark");
    paramGroup->setObjectName("Panel");
    auto* paramLayout = new QGridLayout(paramGroup);
    paramLayout->setSpacing(10);

    paramLayout->addWidget(new QLabel("Dimensiuni grafuri (noduri, separate prin virgula):"), 0, 0);
    nodesInput_ = new QLineEdit("5, 10, 20, 50");
    nodesInput_->setPlaceholderText("ex: 5, 10, 20, 50, 100");
    paramLayout->addWidget(nodesInput_, 0, 1);

    paramLayout->addWidget(new QLabel("Max Capacity Limit (intervalul capacitatilor):"), 1, 0);
    maxCapLimitInput_ = new QSpinBox();
    maxCapLimitInput_->setRange(10, 2000000000);
    maxCapLimitInput_->setValue(500000);
    maxCapLimitInput_->setSingleStep(1000);
    maxCapLimitInput_->setGroupSeparatorShown(true);
    paramLayout->addWidget(maxCapLimitInput_, 1, 1);

    paramLayout->addWidget(new QLabel("Buget W:"), 2, 0);
    budgetInput_ = new QDoubleSpinBox();
    budgetInput_->setRange(1.0, 1e9);
    budgetInput_->setValue(1000.0);
    budgetInput_->setDecimals(2);
    paramLayout->addWidget(budgetInput_, 2, 1);

    rootLayout->addWidget(paramGroup);


    auto* ctrlLayout = new QHBoxLayout();

    runButton_ = new QPushButton("Ruleaza Benchmark");
    runButton_->setObjectName("RunButton");
    ctrlLayout->addWidget(runButton_);

    saveButton_ = new QPushButton("Salveaza CSV");
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


    auto* tableLabel = new QLabel("Rezultate:");
    tableLabel->setObjectName("PanelTitle");
    rootLayout->addWidget(tableLabel);

    resultsTable_ = new QTableWidget(0, 11);
    resultsTable_->setHorizontalHeaderLabels({
        "Noduri", "Arce", "W",
        "AMCPEBCP (ms)", "Apeluri Alg3", "z* Alg3",
        "Param. Search (ms)", "Apeluri Alg4", "z* Alg4",
        "Castigator",
        "\xCE\x94z*"
    });
    resultsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    resultsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTable_->setAlternatingRowColors(true);
    rootLayout->addWidget(resultsTable_, 1);


    auto* logLabel = new QLabel("Log:");
    logLabel->setObjectName("PanelContent");
    rootLayout->addWidget(logLabel);

    logBox_ = new QTextEdit();
    logBox_->setObjectName("LogBox");
    logBox_->setReadOnly(true);
    logBox_->setMaximumHeight(120);
    rootLayout->addWidget(logBox_);


    connect(runButton_, &QPushButton::clicked, this, &BenchmarkWidget::onRunClicked);
    connect(saveButton_, &QPushButton::clicked, this, &BenchmarkWidget::onSaveClicked);
}

void BenchmarkWidget::applyStyle()
{
    setStyleSheet(R"(
        QGroupBox
        {
            border: 1px solid #27446f;
            border-radius: 10px;
            margin-top: 8px;
            padding: 8px;
            color: #e8f1ff;
        }
        QGroupBox::title
        {
            subcontrol-origin: margin;
            left: 10px;
            color: #7dd3fc;
        }
        QTableWidget
        {
            background-color: #060b17;
            border: 1px solid #233b61;
            border-radius: 8px;
            color: #cfe4ff;
            gridline-color: #1a2e4a;
        }
        QTableWidget::item:alternate
        {
            background-color: #0a1628;
        }
        QHeaderView::section
        {
            background-color: #0d1630;
            color: #7dd3fc;
            border: 1px solid #27446f;
            padding: 4px;
            font-weight: bold;
        }
        #RunButton
        {
            background-color: #1a4a8a;
            border: 1px solid #3a7bd5;
            border-radius: 10px;
            padding: 10px 20px;
            color: #ffffff;
            font-weight: 700;
            font-size: 13px;
        }
        #RunButton:hover
        {
            background-color: #2560b0;
        }
        #RunButton:disabled
        {
            background-color: #0d1630;
            color: #555;
        }
    )");
}

void BenchmarkWidget::addTableRow(const BenchmarkEntry& e, int row)
{
    resultsTable_->insertRow(row);

    auto addItem = [&](int col, const QString& text,
                       QColor fg = QColor("#cfe4ff"), bool bold = false)
    {
        auto* item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        item->setForeground(fg);
        if (bold)
        {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }
        resultsTable_->setItem(row, col, item);
    };

    addItem(0, QString::number(e.nodes));
    addItem(1, QString::number(e.edges));
    addItem(2, QString::number(e.W, 'f', 1));


    addItem(3, QString::number(e.amcpebcp_time_ms, 'f', 2));
    addItem(4, QString::number(e.amcpebcp_oracle_calls));
    addItem(5, QString::number(e.amcpebcp_result_z, 'f', 4));


    bool alg4Wins = (e.parametric_time_ms < e.amcpebcp_time_ms);
    QColor paramColor = alg4Wins ? QColor("#2ecc71") : QColor("#cfe4ff");
    addItem(6, QString::number(e.parametric_time_ms, 'f', 2), paramColor);
    addItem(7, QString::number(e.parametric_oracle_calls), paramColor, alg4Wins);
    addItem(8, QString::number(e.parametric_result_z, 'f', 4));


    QString winner = alg4Wins ? "Alg4 (Param)" : "Alg3 (AMCPEBCP)";
    QColor winColor = alg4Wins ? QColor("#2ecc71") : QColor("#7dd3fc");
    addItem(9, winner, winColor, true);


    bool correct = e.z_difference < 1.0;
    addItem(10, QString::number(e.z_difference, 'f', 6),
            correct ? QColor("#2ecc71") : QColor("#e74c3c"));
}

void BenchmarkWidget::setRunning(bool running)
{
    runButton_->setEnabled(!running);
    nodesInput_->setEnabled(!running);
    budgetInput_->setEnabled(!running);
    maxCapLimitInput_->setEnabled(!running);
    progressBar_->setVisible(running);
    saveButton_->setEnabled(!running && !results_.empty());
}

void BenchmarkWidget::onRunClicked()
{
    std::vector<int> sizes;
    QStringList parts = nodesInput_->text().split(',', Qt::SkipEmptyParts);

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

    results_.clear();
    resultsTable_->setRowCount(0);
    logBox_->clear();
    progressBar_->setRange(0, (int)sizes.size());
    progressBar_->setValue(0);
    setRunning(true);

    double W = budgetInput_->value();
    int maxCapLimit = maxCapLimitInput_->value();

    logBox_->append(QString("Pornire benchmark: %1 dimensiuni, W=%2, MaxCapLimit=%3")
        .arg(sizes.size()).arg(W).arg(maxCapLimit));

    workerThread_ = new QThread(this);
    auto* worker = new BenchmarkWorker(sizes, W, maxCapLimit, 42);
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

    bool alg4Wins = (entry.parametric_time_ms < entry.amcpebcp_time_ms);
    logBox_->append(
        QString("n=%1 | AMCPEBCP: %2ms (%3 apeluri, z*=%4) | Param: %5ms (%6 apeluri, z*=%7) | Castigator: %8 | dz=%9")
        .arg(entry.nodes)
        .arg(entry.amcpebcp_time_ms, 0, 'f', 2)
        .arg(entry.amcpebcp_oracle_calls)
        .arg(entry.amcpebcp_result_z, 0, 'f', 4)
        .arg(entry.parametric_time_ms, 0, 'f', 2)
        .arg(entry.parametric_oracle_calls)
        .arg(entry.parametric_result_z, 0, 'f', 4)
        .arg(alg4Wins ? "Alg4" : "Alg3")
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
        this, "Salveaza rezultate benchmark", "benchmark_results.csv",
        "CSV Files (*.csv)"
    );

    if (fileName.isEmpty())
        return;

    PerformanceTest test;
    test.saveToCSV(results_, fileName.toStdString());
    logBox_->append("Rezultate salvate: " + fileName);
}

