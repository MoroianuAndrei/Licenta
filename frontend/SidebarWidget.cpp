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

    generateButton->setStyleSheet(buttonStyle);
    loadRunButton->setStyleSheet(buttonStyle);

    layout->addWidget(title);
    layout->addSpacing(24);
    layout->addWidget(generateButton);
    layout->addWidget(loadRunButton);
    layout->addStretch();

    setStyleSheet("background-color: #070b18;");

    connect(generateButton, &QPushButton::clicked,
        this, &SidebarWidget::generateGraphClicked);

    connect(loadRunButton, &QPushButton::clicked,
        this, &SidebarWidget::loadRunClicked);
}