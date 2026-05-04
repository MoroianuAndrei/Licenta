#include "GraphWidget.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
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
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace
{
    constexpr double kNodeRadius = 22.0;

    QPainterPath arrowHead(const QPointF& start, const QPointF& end, double size)
    {
        const double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
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


    class ZoomableGraphicsView final : public QGraphicsView
    {
    public:
        explicit ZoomableGraphicsView(QGraphicsScene* scene, GraphWidget* owner, QWidget* parent = nullptr)
            : QGraphicsView(scene, parent), owner_(owner)
        {
            setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            setResizeAnchor(QGraphicsView::AnchorUnderMouse);
            setDragMode(QGraphicsView::NoDrag);
            setMouseTracking(true);
        }

    protected:
        void wheelEvent(QWheelEvent* event) override
        {
            if (event->modifiers() & Qt::ControlModifier) {
                const double zoomFactor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
                scale(zoomFactor, zoomFactor);
                if (owner_)
                    owner_->refreshViewportItems();
                event->accept();
                return;
            }

            QGraphicsView::wheelEvent(event);
        }

        void mousePressEvent(QMouseEvent* event) override
        {
            if (event->button() == Qt::MiddleButton) {
                isPanning_ = true;
                lastPanPoint_ = event->pos();
                setCursor(Qt::ClosedHandCursor);
                event->accept();
                return;
            }

            QGraphicsView::mousePressEvent(event);
        }

        void mouseMoveEvent(QMouseEvent* event) override
        {
            if (isPanning_) {
                const QPoint delta = event->pos() - lastPanPoint_;
                lastPanPoint_ = event->pos();

                horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
                verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());

                event->accept();
                return;
            }

            QGraphicsView::mouseMoveEvent(event);
        }

        void mouseReleaseEvent(QMouseEvent* event) override
        {
            if (event->button() == Qt::MiddleButton && isPanning_) {
                isPanning_ = false;
                setCursor(Qt::ArrowCursor);
                event->accept();
                return;
            }

            QGraphicsView::mouseReleaseEvent(event);
        }

    private:
        GraphWidget* owner_ = nullptr;
        bool isPanning_ = false;
        QPoint lastPanPoint_;
    };

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
    view_ = new ZoomableGraphicsView(scene_, this, this);
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
    autoFitPending_ = true;
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


double GraphWidget::currentViewScale() const
{
    if (!view_)
        return 1.0;

    const double scale = std::abs(view_->transform().m11());
    return std::max(0.001, scale);
}

void GraphWidget::refreshViewportItems()
{
    redraw();
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
        if (autoFitPending_) {
            view_->resetTransform();
            view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
            autoFitPending_ = false;
        }
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

    const double viewScale = currentViewScale();
    const double nodeOffset = kNodeRadius / viewScale;
    const double arrowSize = 12.0 / viewScale;

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

        QPointF start = a + dir * nodeOffset;
        QPointF end = b - dir * nodeOffset;

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
        pen.setCosmetic(true);

        QPainterPath path;
        path.moveTo(start);
        path.lineTo(end);
        auto* line = scene_->addPath(path, pen);
        line->setZValue(selected ? 3 : 1);

        auto* arrow = scene_->addPath(arrowHead(start, end, arrowSize), pen, QBrush(color));
        arrow->setZValue(selected ? 4 : 2);

        QPainterPathStroker stroker;
        stroker.setWidth(18.0 / viewScale);
        auto* clickable = new EdgeItem(this, edge, stroker.createStroke(path));
        clickable->setPen(Qt::NoPen);
        clickable->setBrush(Qt::NoBrush);
        scene_->addItem(clickable);

    }

    for (int node : nodes) {
        const QPointF p = pos[node];
        QRectF rect(-kNodeRadius, -kNodeRadius, kNodeRadius * 2.0, kNodeRadius * 2.0);
        QPen nodePen(QColor(110, 190, 255), 2.0);
        nodePen.setCosmetic(true);

        auto* circle = scene_->addEllipse(rect, nodePen, QBrush(QColor(22, 63, 108)));
        circle->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        circle->setPos(p);
        circle->setZValue(10);

        auto* text = scene_->addText(QString::number(node));
        text->setDefaultTextColor(Qt::white);
        text->setFont(QFont("Segoe UI", 11, QFont::Bold));
        text->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        QRectF br = text->boundingRect();
        text->setPos(p.x() - br.width() / 2.0, p.y() - br.height() / 2.0);
        text->setZValue(11);
    }

    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-80, -80, 80, 80));
    if (autoFitPending_) {
        view_->resetTransform();
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
        autoFitPending_ = false;
    }
}
