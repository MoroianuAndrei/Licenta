#pragma once

#include <QWidget>
#include <QString>
#include <vector>

class QLabel;
class QTextEdit;

class ResultWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResultWidget(QWidget* parent = nullptr);

public slots:
    void clearResult();
    void setResult(const QString& algorithm, const std::vector<int>& path, double zValue, double finalCost, bool feasible);
    void appendLog(const QString& text);

private:
    QString formatPath(const std::vector<int>& path) const;
    QString formatNumber(double value) const;

    QLabel* statusLabel_ = nullptr;
    QLabel* pathLabel_ = nullptr;
    QLabel* costLabel_ = nullptr;
    QTextEdit* log_ = nullptr;
};
