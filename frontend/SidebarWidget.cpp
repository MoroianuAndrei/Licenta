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

    generateButton_ = new QPushButton("Generate Graph CSV");
    loadRunButton_ = new QPushButton("Load & Run CSV");
    benchmarkButton_ = new QPushButton("Benchmark");
    testsButton_ = new QPushButton("Tests & Validation");

    layout->addWidget(title);
    layout->addSpacing(24);
    layout->addWidget(generateButton_);
    layout->addWidget(loadRunButton_);
    layout->addWidget(benchmarkButton_);
    layout->addWidget(testsButton_);
    layout->addStretch();

    setStyleSheet("background-color: #070b18;");

    applyButtonStyles();

    connect(generateButton_, &QPushButton::clicked, this, [this]() {
        setActiveButton(0);
        emit generateGraphClicked();
    });

    connect(loadRunButton_, &QPushButton::clicked, this, [this]() {
        setActiveButton(1);
        emit loadRunClicked();
    });

    connect(benchmarkButton_, &QPushButton::clicked, this, [this]() {
        setActiveButton(2);
        emit benchmarkClicked();
    });

    connect(testsButton_, &QPushButton::clicked, this, [this]() {
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
    generateButton_->setStyleSheet(activeIndex_ == 0 ? kActiveStyle : kNormalStyle);
    loadRunButton_->setStyleSheet(activeIndex_ == 1 ? kActiveStyle : kNormalStyle);
    benchmarkButton_->setStyleSheet(activeIndex_ == 2 ? kActiveStyle : kNormalStyle);
    testsButton_->setStyleSheet(activeIndex_ == 3 ? kActiveStyle : kNormalStyle);
}

