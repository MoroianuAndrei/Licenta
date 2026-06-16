#include "TestWidget.h"

#include "backend/testing/ManualTests.h"
#include "backend/testing/ValidationTests.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QFont>

#include <iostream>
#include <sstream>

TestWidget::TestWidget(QWidget* parent)
    : QWidget(parent)
{
    buildUi();
    applyStyle();
}

void TestWidget::buildUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto* title = new QLabel("Validare & Teste");
    title->setObjectName("PanelTitle");
    layout->addWidget(title);

    auto* desc = new QLabel(
        "Teste manuale pe exemplele din paper (Fig.1, Fig.5) + validare brute-force\n"
        "pe 50 grafuri random. Verifica corectitudinea algoritmilor AMCMCPEP, AMCPEBCP si ParametricSearch."
    );
    desc->setObjectName("PanelContent");
    desc->setWordWrap(true);
    layout->addWidget(desc);

    runButton_ = new QPushButton("Run All Tests");
    runButton_->setObjectName("RunButton");
    layout->addWidget(runButton_);

    summaryLabel_ = new QLabel("");
    summaryLabel_->setObjectName("PanelContent");
    layout->addWidget(summaryLabel_);

    logBox_ = new QTextEdit();
    logBox_->setObjectName("LogBox");
    logBox_->setReadOnly(true);
    logBox_->setFont(QFont("Consolas", 10));
    layout->addWidget(logBox_, 1);

    connect(runButton_, &QPushButton::clicked, this, &TestWidget::onRunClicked);
}

void TestWidget::applyStyle()
{
    setStyleSheet(R"(
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
        #RunButton:hover { background-color: #2560b0; }
        #RunButton:disabled { background-color: #0d1630; color: #555; }
    )");
}

void TestWidget::onRunClicked()
{
    runButton_->setEnabled(false);
    logBox_->clear();
    summaryLabel_->setText("Se ruleaza testele...");

    std::ostringstream oss;
    std::streambuf* oldBuf = std::cout.rdbuf(oss.rdbuf());

    try
    {
        runAllManualTests();
        runAllValidationTests();
    } catch (...) {}

    std::cout.rdbuf(oldBuf);

    QString output = QString::fromStdString(oss.str());
    displayOutput(output);

    int correct = output.count("CORECT");
    int wrong = output.count("DIFERIT") + output.count("NECONCORDANTA") + output.count("GRESIT");

    summaryLabel_->setText(
        QString("Rezultat: %1 corecte |  %2 erori").arg(correct).arg(wrong)
    );

    runButton_->setEnabled(true);
}

void TestWidget::displayOutput(const QString& text)
{
    QTextCursor cursor = logBox_->textCursor();
    const QFont mono("Consolas", 10);

    const QColor colorDefault("#cfe4ff");
    const QColor colorGreen("#2ecc71");
    const QColor colorRed("#e74c3c");
    const QColor colorCyan("#7dd3fc");

    for (const QString& line : text.split('\n'))
    {
        QTextCharFormat fmt;
        fmt.setFont(mono);

        if (line.contains("CORECT"))
            fmt.setForeground(colorGreen);
        else if (line.contains("DIFERIT") || line.contains("NECONCORDANTA") || line.contains("GRESIT"))
            fmt.setForeground(colorRed);
        else if (line.startsWith("=") || line.startsWith("*"))
            fmt.setForeground(colorCyan);
        else
            fmt.setForeground(colorDefault);

        cursor.insertText(line + "\n", fmt);
    }

    logBox_->setTextCursor(cursor);
    logBox_->moveCursor(QTextCursor::Start);
}

