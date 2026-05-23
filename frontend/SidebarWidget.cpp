#include "SidebarWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

SidebarWidget::SidebarWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(240);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 28, 24, 24);
    layout->setSpacing(16);

    auto* title = new QLabel("Network\nOptimizer");
    title->setStyleSheet(
        "font-size: 28px;"
        "font-weight: 800;"
        "color: white;"
    );

    generateButton = new QPushButton("Generate Graph CSV");
    loadRunButton = new QPushButton("Load & Run CSV");
    benchmarkButton = new QPushButton("Benchmark");

    QString buttonStyle =
        "QPushButton {"
        "background-color: #1a1f2e;"
        "color: white;"
        "border: none;"
        "border-radius: 12px;"
        "padding: 14px;"
        "font-size: 15px;"
        "text-align: left;"
        "}"
        "QPushButton:hover {"
        "background-color: #263044;"
        "}";

    QString benchmarkStyle =
        "QPushButton {"
        "background-color: #1a3a5c;"
        "color: #7dd3fc;"
        "border: 1px solid #27446f;"
        "border-radius: 12px;"
        "padding: 14px;"
        "font-size: 15px;"
        "font-weight: 700;"
        "text-align: left;"
        "}"
        "QPushButton:hover {"
        "background-color: #1e4a78;"
        "color: #bae6fd;"
        "}";

    generateButton->setStyleSheet(buttonStyle);
    loadRunButton->setStyleSheet(buttonStyle);
    benchmarkButton->setStyleSheet(benchmarkStyle);

    layout->addWidget(title);
    layout->addSpacing(24);
    layout->addWidget(generateButton);
    layout->addWidget(loadRunButton);
    layout->addWidget(benchmarkButton);
    layout->addStretch();

    setStyleSheet("background-color: #070b18;");

    connect(generateButton, &QPushButton::clicked,
        this, &SidebarWidget::generateGraphClicked);

    connect(loadRunButton, &QPushButton::clicked,
        this, &SidebarWidget::loadRunClicked);

    connect(benchmarkButton, &QPushButton::clicked,
        this, &SidebarWidget::benchmarkClicked);
}