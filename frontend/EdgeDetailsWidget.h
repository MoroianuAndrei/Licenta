#pragma once

#include "UiModels.h"

#include <QWidget>
#include <optional>

class QLabel;

class EdgeDetailsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EdgeDetailsWidget(QWidget* parent = nullptr);

public slots:
    void setContext(const AlgorithmViewState& state);
    void clearEdge();
    void setEdge(const VisualEdge& edge);

private:
    void render();
    QString costDetails(const VisualEdge& edge) const;

    QLabel* title_   = nullptr;
    QLabel* content_ = nullptr;
    AlgorithmViewState state_;
    std::optional<VisualEdge> selectedEdge_;
};

