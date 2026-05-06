#include "GraphWidget.h"

#include <QBrush>
#include <QFont>
#include <QFrame>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPen>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace
{
    constexpr double kBaseNodeRadius = 22.0;   // visual px
    constexpr double kBaseArrowSize = 13.0;    // visual px
    constexpr double kPi = 3.14159265358979323846;

    bool sameEdge(const VisualEdge& a, const VisualEdge& b)
    {
        return a.from == b.from && a.to == b.to;
    }

    QPainterPath makeArrowHead(const QPointF& start, const QPointF& end, double size)
    {
        const double angle = std::atan2(end.y() - start.y(), end.x() - start.x());

        const QPointF p1 = end - QPointF(
            std::cos(angle - kPi / 6.0) * size,
            std::sin(angle - kPi / 6.0) * size
        );

        const QPointF p2 = end - QPointF(
            std::cos(angle + kPi / 6.0) * size,
            std::sin(angle + kPi / 6.0) * size
        );

        QPainterPath path;
        path.moveTo(end);
        path.lineTo(p1);
        path.lineTo(p2);
        path.closeSubpath();
        return path;
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
            setZValue(20);
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

void GraphWidget::setLargeGraphMode(bool enabled)
{
    largeGraphMode_ = enabled;
    redraw();
}

QString GraphWidget::formatNumber(double value) const
{
    if (std::abs(value - std::round(value)) < 1e-6)
        return QString::number(static_cast<int>(std::round(value)));

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
        scene_->setSceneRect(0, 0, 720, 440);

        if (autoFitPending_) {
            view_->resetTransform();
            view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
            autoFitPending_ = false;
        }
        return;
    }

    if (largeGraphMode_) {
        auto* text = scene_->addText(
            "Large graph loaded.\n"
            "Graph visualization is disabled for performance.\n"
            "Use the generated CSV for algorithm benchmarks."
        );
        text->setDefaultTextColor(QColor(220, 235, 255));
        text->setFont(QFont("Segoe UI", 14, QFont::Bold));
        text->setTextWidth(520);
        text->setPos(80, 80);

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
    const double radius = std::max(210.0, nodes.size() * 6.0);
    int index = 0;
    const int n = std::max(1, static_cast<int>(nodes.size()));

    for (int node : nodes) {
        const double angle = -kPi / 2.0 + 2.0 * kPi * index / n;
        pos[node] = QPointF(
            center.x() + radius * std::cos(angle),
            center.y() + radius * std::sin(angle)
        );
        ++index;
    }

    QRectF layoutRect;
    for (const auto& item : pos) {
        if (layoutRect.isNull())
            layoutRect = QRectF(item.second, QSizeF(1, 1));
        else
            layoutRect = layoutRect.united(QRectF(item.second, QSizeF(1, 1)));
    }

    scene_->setSceneRect(layoutRect.adjusted(-120, -120, 120, 120));

    if (autoFitPending_) {
        view_->resetTransform();
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
        autoFitPending_ = false;
    }

    const double scale = currentViewScale();
    const double invScale = 1.0 / scale;

    const double nodeRadius = kBaseNodeRadius * invScale;
    const double arrowSize = kBaseArrowSize * invScale;
    const double clickableWidth = 18.0 * invScale;

    for (const auto& edge : edges_) {
        const bool used = state_.hasResult && edgeIsInPath(edge, state_.path);
        const bool unavailable = state_.hasResult && !used && edge.maxCapacity + 1e-9 < state_.resultZ;
        if (unavailable)
            continue;

        const bool selected = selectedEdge_ && sameEdge(edge, *selectedEdge_);

        const QPointF a = pos[edge.from];
        const QPointF b = pos[edge.to];
        QPointF dir = b - a;
        const double len = std::hypot(dir.x(), dir.y());
        if (len <= 1e-9)
            continue;
        dir /= len;

        const QPointF start = a + dir * nodeRadius;
        const QPointF end = b - dir * nodeRadius;

        QColor color = edge.isNew ? QColor(145, 135, 160) : QColor(122, 137, 158);
        double visualWidth = 2.2;

        if (used) {
            color = edge.isNew ? QColor(38, 225, 128) : QColor(25, 150, 255);
            visualWidth = 4.0;
        }

        if (selected) {
            color = used
                ? (edge.isNew ? QColor(96, 255, 168) : QColor(90, 190, 255))
                : QColor(205, 215, 230);
            visualWidth += 3.0;
        }

        QPen pen(color, visualWidth * invScale, edge.isNew ? Qt::DashLine : Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        QPainterPath path;
        path.moveTo(start);
        path.lineTo(end);

        auto* line = scene_->addPath(path, pen);
        line->setZValue(selected ? 4 : 1);

        auto* arrow = scene_->addPath(makeArrowHead(start, end, arrowSize), pen, QBrush(color));
        arrow->setZValue(selected ? 5 : 2);

        QPainterPathStroker stroker;
        stroker.setWidth(clickableWidth);
        auto* clickable = new EdgeItem(this, edge, stroker.createStroke(path));
        clickable->setPen(Qt::NoPen);
        clickable->setBrush(Qt::NoBrush);
        scene_->addItem(clickable);
    }

    const bool showNodeLabels = nodes.size() <= 200;

    for (int node : nodes) {
        const QPointF p = pos[node];

        // The node item ignores the view transformation, so it keeps the same
        // visual size while zooming. Its local coordinate system is centered
        // in (0, 0), and the item is positioned at the logical node position.
        auto* circle = scene_->addEllipse(
            -kBaseNodeRadius,
            -kBaseNodeRadius,
            kBaseNodeRadius * 2.0,
            kBaseNodeRadius * 2.0,
            QPen(QColor(110, 190, 255), 2.0),
            QBrush(QColor(22, 63, 108))
        );
        circle->setPos(p);
        circle->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        circle->setZValue(10);

        if (showNodeLabels) {
            auto* text = new QGraphicsSimpleTextItem(QString::number(node), circle);
            text->setBrush(QBrush(Qt::white));
            text->setFont(QFont("Segoe UI", 11, QFont::Bold));

            const QRectF br = text->boundingRect();
            text->setPos(-br.width() / 2.0, -br.height() / 2.0);
            text->setZValue(11);
        }
    }

}
