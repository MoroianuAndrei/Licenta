#pragma once

#include "UiModels.h"

#include <QWidget>
#include <map>
#include <optional>
#include <set>
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
    void refreshViewportItems();
    void setLargeGraphMode(bool enabled);

signals:
    void edgeSelected(const VisualEdge& edge);

private:
    void redraw();
    void handleSpecialModes();
    std::map<int, QPointF> computeCircularLayout(const std::set<int>& nodes) const;
    void drawEdges(const std::map<int, QPointF>& pos, double invScale);
    void drawNodes(const std::map<int, QPointF>& pos, const std::set<int>& nodes);
    double currentViewScale() const;

    QGraphicsScene* scene_ = nullptr;
    QGraphicsView* view_ = nullptr;

    std::vector<VisualEdge> edges_;
    AlgorithmViewState state_;
    std::optional<VisualEdge> selectedEdge_;

    bool autoFitPending_ = true;
    bool largeGraphMode_ = false;
};

