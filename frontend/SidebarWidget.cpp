#include "SidebarWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

static const QString kNormalStyle =
    "QPushButton {"
    " background-color: #1a1f2e;"
    " color: #c8d8f0;"
    " border: none;"
    " border-radius: 12px;"
    " padding: 14px;"
    " font-size: 15px;"
    " text-align: left;"
    "}"
    "QPushButton:hover {"
    " background-color: #263044;"
    " color: white;"
    "}";

static const QString kActiveStyle =
    "QPushButton {"
    " background-color: #1a3a5c;"
    " color: #7dd3fc;"
    " border: 1px solid #27446f;"
    " border-radius: 12px;"
    " padding: 14px;"
    " font-size: 15px;"
    " font-weight: 700;"
    " text-align: left;"
    "}"
    "QPushButton:hover {"
    " background-color: #1e4a78;"
    " color: #bae6fd;"
    "}";

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
    testsButton = new QPushButton("Tests & Validation");

    layout->addWidget(title);
    layout->addSpacing(24);
    layout->addWidget(generateButton);
    layout->addWidget(loadRunButton);
    layout->addWidget(benchmarkButton);
    layout->addWidget(testsButton);
    layout->addStretch();

    setStyleSheet("background-color: #070b18;");

    applyButtonStyles();

    connect(generateButton, &QPushButton::clicked, this, [this]() {
        setActiveButton(0);
        emit generateGraphClicked();
    });

    connect(loadRunButton, &QPushButton::clicked, this, [this]() {
        setActiveButton(1);
        emit loadRunClicked();
    });

    connect(benchmarkButton, &QPushButton::clicked, this, [this]() {
        setActiveButton(2);
        emit benchmarkClicked();
    });

    connect(testsButton, &QPushButton::clicked, this, [this]() {
        setActiveButton(3);
        emit testsClicked();
    });
}

void SidebarWidget::setActiveButton(int index)
{
    activeIndex_ = index;
    applyButtonStyles();
}

void SidebarWidget::applyButtonStyles()
{
    generateButton ->setStyleSheet(activeIndex_ == 0 ? kActiveStyle : kNormalStyle);
    loadRunButton ->setStyleSheet(activeIndex_ == 1 ? kActiveStyle : kNormalStyle);
    benchmarkButton->setStyleSheet(activeIndex_ == 2 ? kActiveStyle : kNormalStyle);
    testsButton ->setStyleSheet(activeIndex_ == 3 ? kActiveStyle : kNormalStyle);
}

