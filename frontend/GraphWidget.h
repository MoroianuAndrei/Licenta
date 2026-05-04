#pragma once

#include "UiModels.h"

#include <QWidget>
#include <optional>
#include <vector>

class QGraphicsScene;
class QGraphicsView;

class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWidget(QWidget* parent = nullptr);

    void setEdges(const std::vector<VisualEdge>& edges);
    void setViewState(const AlgorithmViewState& state);
    void clearSelection();
    void selectEdge(const VisualEdge& edge);

signals:
    void edgeSelected(const VisualEdge& edge);

private:
    void redraw();
    QString formatNumber(double value) const;
    QString shortEdgeLabel(const VisualEdge& edge) const;

    QGraphicsScene* scene_ = nullptr;
    QGraphicsView* view_ = nullptr;
    std::vector<VisualEdge> edges_;
    AlgorithmViewState state_;
    std::optional<VisualEdge> selectedEdge_;
};
