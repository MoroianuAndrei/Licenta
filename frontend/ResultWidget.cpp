#include "ResultWidget.h"
#include "UiModels.h"

#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>

ResultWidget::ResultWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("Panel");
    setMinimumHeight(160);

    auto* title = new QLabel("Rezultat curent", this);
    title->setObjectName("PanelTitle");

    statusLabel_ = new QLabel("Status: asteapta rulare", this);
    pathLabel_ = new QLabel("Ruta optima: -", this);
    costLabel_ = new QLabel("Cost final: -", this);

    for (auto* label : {statusLabel_, pathLabel_, costLabel_})
    {
        label->setObjectName("PanelContent");
        label->setWordWrap(true);
    }

    log_ = new QTextEdit(this);
    log_->setReadOnly(true);
    log_->setObjectName("LogBox");
    log_->setMaximumHeight(70);

    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setHorizontalSpacing(18);
    layout->setVerticalSpacing(8);
    layout->addWidget(title, 0, 0, 1, 2);
    layout->addWidget(pathLabel_, 1, 0);
    layout->addWidget(costLabel_, 2, 0);
    layout->addWidget(statusLabel_,3, 0);
    layout->addWidget(log_, 1, 1, 3, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
}

void ResultWidget::clearResult()
{
    statusLabel_->setText("Status: asteapta rulare");
    pathLabel_->setText("Ruta optima: -");
    costLabel_->setText("Cost final: -");
}

void ResultWidget::setResult(const QString& algorithm, const std::vector<int>& path,
                             double zValue, double finalCost, bool feasible)
{
    Q_UNUSED(algorithm);
    pathLabel_->setText("Ruta optima: " + formatPath(path));
    costLabel_->setText(QString("z*: %1 |   Cost final: %2")
        .arg(formatNumber(zValue), formatNumber(finalCost)));
    statusLabel_->setText(feasible ? "Status: solutie gasita"
                                   : "Status: problema infezabila / fara drum");
}

void ResultWidget::appendLog(const QString& text)
{
    log_->append(text);
    log_->ensureCursorVisible();
}

