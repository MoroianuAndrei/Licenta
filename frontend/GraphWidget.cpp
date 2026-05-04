#include "GraphWidget.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPen>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace
{
    constexpr double kNodeRadius = 22.0;

    QPainterPath arrowHead(const QPointF& start, const QPointF& end)
    {
        const double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
        const double size = 12.0;
        QPointF p1 = end - QPointF(std::cos(angle - 3.14159265358979323846 / 6.0) * size, std::sin(angle - 3.14159265358979323846 / 6.0) * size);
        QPointF p2 = end - QPointF(std::cos(angle + 3.14159265358979323846 / 6.0) * size, std::sin(angle + 3.14159265358979323846 / 6.0) * size);

        QPainterPath path;
        path.moveTo(end);
        path.lineTo(p1);
        path.lineTo(p2);
        path.closeSubpath();
        return path;
    }

    bool sameEdge(const VisualEdge& a, const VisualEdge& b)
    {
        return a.from == b.from && a.to == b.to;
    }

    class EdgeItem final : public QGraphicsPathItem
    {
    public:
        EdgeItem(GraphWidget* owner, const VisualEdge& edge, const QPainterPath& path)
            : QGraphicsPathItem(path), owner_(owner), edge_(edge)
        {
            setAcceptedMouseButtons(Qt::LeftButton);
            setAcceptHoverEvents(true);
            setCursor(Qt::PointingHandCursor);
            setZValue(5);
        }

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override
        {
            Q_UNUSED(event);
            if (owner_)
                owner_->selectEdge(edge_);
        }

    private:
        GraphWidget* owner_ = nullptr;
        VisualEdge edge_;
    };
}

GraphWidget::GraphWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("Panel");

    scene_ = new QGraphicsScene(this);
    view_ = new QGraphicsView(scene_, this);
    view_->setRenderHint(QPainter::Antialiasing);
    view_->setBackgroundBrush(QBrush(QColor(8, 13, 30)));
    view_->setFrameShape(QFrame::NoFrame);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(view_);

    redraw();
}

void GraphWidget::setEdges(const std::vector<VisualEdge>& edges)
{
    edges_ = edges;
    clearSelection();
    redraw();
}

void GraphWidget::setViewState(const AlgorithmViewState& state)
{
    state_ = state;
    redraw();
}

void GraphWidget::clearSelection()
{
    selectedEdge_.reset();
}

void GraphWidget::selectEdge(const VisualEdge& edge)
{
    selectedEdge_ = edge;
    emit edgeSelected(edge);
    redraw();
}

QString GraphWidget::formatNumber(double value) const
{
    if (std::abs(value - std::round(value)) < 1e-6)
        return QString::number((int)std::round(value));
    return QString::number(value, 'f', 2);
}

QString GraphWidget::shortEdgeLabel(const VisualEdge& edge) const
{
    return QString("%1 / %2")
        .arg(formatNumber(edge.capacity), formatNumber(edge.maxCapacity));
}

void GraphWidget::redraw()
{
    scene_->clear();

    if (edges_.empty()) {
        auto* text = scene_->addText("Importa un fisier CSV pentru a vizualiza graful.");
        text->setDefaultTextColor(QColor(190, 205, 225));
        text->setFont(QFont("Segoe UI", 12, QFont::Bold));
        text->setPos(40, 40);
        scene_->setSceneRect(0, 0, 720, 440);
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
        return;
    }

    std::set<int> nodes;
    for (const auto& e : edges_) {
        nodes.insert(e.from);
        nodes.insert(e.to);
    }

    std::map<int, QPointF> pos;
    const QPointF center(390.0, 260.0);
    const double radius = 205.0;
    int index = 0;
    const int n = std::max(1, (int)nodes.size());

    for (int node : nodes) {
        const double angle = -3.14159265358979323846 / 2.0 + 2.0 * 3.14159265358979323846 * index / n;
        pos[node] = QPointF(center.x() + radius * std::cos(angle), center.y() + radius * std::sin(angle));
        ++index;
    }

    auto* title = scene_->addText(state_.hasResult
        ? QString("Solutie curenta: z*=%1 | click pe muchie pentru detalii").arg(formatNumber(state_.resultZ))
        : "Graf incarcat | click pe muchie pentru detalii");
    title->setDefaultTextColor(QColor(220, 235, 255));
    title->setFont(QFont("Segoe UI", 11, QFont::Bold));
    title->setPos(18, 16);

    for (const auto& edge : edges_) {
        const bool used = state_.hasResult && edgeIsInPath(edge, state_.path);
        const bool unavailable = state_.hasResult && !used && edge.maxCapacity + 1e-9 < state_.resultZ;
        if (unavailable)
            continue;

        const bool selected = selectedEdge_ && sameEdge(edge, *selectedEdge_);

        QPointF a = pos[edge.from];
        QPointF b = pos[edge.to];
        QPointF dir = b - a;
        const double len = std::hypot(dir.x(), dir.y());
        if (len <= 1e-9)
            continue;
        dir /= len;

        QPointF start = a + dir * kNodeRadius;
        QPointF end = b - dir * kNodeRadius;

        QColor color = edge.isNew ? QColor(145, 135, 160) : QColor(122, 137, 158);
        double width = 2.2;

        if (used) {
            color = edge.isNew ? QColor(38, 225, 128) : QColor(25, 150, 255);
            width = 4.0;
        }

        if (selected) {
            if (used)
                color = edge.isNew ? QColor(96, 255, 168) : QColor(90, 190, 255);
            else
                color = QColor(205, 215, 230);
            width += 3.0;
        }

        QPen pen(color, width, edge.isNew ? Qt::DashLine : Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        QPainterPath path;
        path.moveTo(start);
        path.lineTo(end);
        auto* line = scene_->addPath(path, pen);
        line->setZValue(selected ? 3 : 1);

        auto* arrow = scene_->addPath(arrowHead(start, end), pen, QBrush(color));
        arrow->setZValue(selected ? 4 : 2);

        QPainterPathStroker stroker;
        stroker.setWidth(18.0);
        auto* clickable = new EdgeItem(this, edge, stroker.createStroke(path));
        clickable->setPen(Qt::NoPen);
        clickable->setBrush(Qt::NoBrush);
        scene_->addItem(clickable);

        QPointF mid = (start + end) / 2.0;
        QPointF normal(-dir.y(), dir.x());
        QPointF labelPos = mid + normal * 22.0;

        auto* label = scene_->addText(shortEdgeLabel(edge));
        label->setDefaultTextColor(color.lighter(selected ? 145 : 115));
        label->setFont(QFont("Segoe UI", 8, QFont::Bold));
        label->setPos(labelPos);
        label->setZValue(6);
    }

    for (int node : nodes) {
        const QPointF p = pos[node];
        QRectF rect(p.x() - kNodeRadius, p.y() - kNodeRadius, kNodeRadius * 2.0, kNodeRadius * 2.0);
        auto* circle = scene_->addEllipse(rect, QPen(QColor(110, 190, 255), 2.0), QBrush(QColor(22, 63, 108)));
        circle->setZValue(10);

        auto* text = scene_->addText(QString::number(node));
        text->setDefaultTextColor(Qt::white);
        text->setFont(QFont("Segoe UI", 11, QFont::Bold));
        QRectF br = text->boundingRect();
        text->setPos(p.x() - br.width() / 2.0, p.y() - br.height() / 2.0);
        text->setZValue(11);
    }

    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-50, -50, 50, 50));
    view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}
