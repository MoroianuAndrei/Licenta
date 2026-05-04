#include "EdgeDetailsWidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <cmath>

EdgeDetailsWidget::EdgeDetailsWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("Panel");
    setMinimumWidth(250);

    title_ = new QLabel("Detalii muchie", this);
    title_->setObjectName("PanelTitle");

    content_ = new QLabel(this);
    content_->setObjectName("PanelContent");
    content_->setWordWrap(true);
    content_->setTextFormat(Qt::RichText);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addWidget(title_);
    layout->addWidget(content_, 1);

    render();
}

void EdgeDetailsWidget::setContext(const AlgorithmViewState& state)
{
    state_ = state;
    render();
}

void EdgeDetailsWidget::clearEdge()
{
    selectedEdge_.reset();
    render();
}

void EdgeDetailsWidget::setEdge(const VisualEdge& edge)
{
    selectedEdge_ = edge;
    render();
}

QString EdgeDetailsWidget::formatNumber(double value) const
{
    if (std::abs(value - std::round(value)) < 1e-6)
        return QString::number((int)std::round(value));
    return QString::number(value, 'f', 2);
}

QString EdgeDetailsWidget::costDetails(const VisualEdge& edge) const
{
    const double z = state_.hasResult ? state_.resultZ : state_.inputValue;

    if (state_.algorithm == "AMCMCPEP") {
        const double d = std::abs(edge.to - edge.from);
        double b = 0.0;
        double u = 0.0;

        if (!edge.isNew) {
            const double diff = std::max(0.0, z - edge.capacity);
            b = 10.0 * d * diff;
            u = d * diff * diff;
        } else {
            b = 4.0 * d * z;
            const double diff = z - 10.0;
            u = d * diff * diff;
        }

        return QString(
            "<hr>"
            "<b>Costuri pentru z = %1</b><br>"
            "b(w,z): <b>%2</b><br>"
            "u(w,z): <b>%3</b><br>"
        ).arg(formatNumber(z), formatNumber(b), formatNumber(u));
    }

    if (state_.algorithm == "Parametric Search") {
        const double linearCost = edge.rate * std::max(0.0, z - edge.capacity);
        return QString(
            "<hr>"
            "<b>Caz liniar</b><br>"
            "b(w,z)=r<sub>w</sub>(z-c(w))<br>"
            "u(w,z)=0<br>"
            "Cost la z=%1: <b>%2</b><br>"
        ).arg(formatNumber(z), formatNumber(linearCost));
    }

    QString html = QString("<hr><b>Buget W:</b> %1<br>").arg(formatNumber(state_.inputValue));
    if (state_.hasResult)
        html += QString("<b>z* calculat:</b> %1<br>").arg(formatNumber(state_.resultZ));
    return html;
}

void EdgeDetailsWidget::render()
{
    if (!selectedEdge_) {
        content_->setText(
            "<span style='color:#b8c7dc'>Selecteaza o muchie din graf pentru a vedea detaliile.</span><br><br>"
            "<b>Legenda:</b><br>"
            "<span style='color:#6aa7ff'>albastru</span> = muchie folosita in solutie<br>"
            "<span style='color:#68e59b'>verde punctat</span> = muchie noua folosita<br>"
            "gri = muchie disponibila/nefolosita<br>"
            "highlight = muchie selectata"
        );
        return;
    }

    const VisualEdge& edge = *selectedEdge_;
    const bool used = state_.hasResult && edgeIsInPath(edge, state_.path);

    QString html;
    html += QString("<b>De la:</b> %1<br>").arg(edge.from);
    html += QString("<b>Catre:</b> %1<br>").arg(edge.to);
    html += QString("<b>Tip:</b> %1<br>").arg(edge.isNew ? "arc potential nou" : "arc existent");
    html += QString("<b>Stare:</b> %1<br><br>").arg(used ? "folosit in solutie" : "disponibil / nefolosit");
    html += QString("<b>Capacitate c(w):</b> %1<br>").arg(formatNumber(edge.capacity));
    html += QString("<b>Capacitate maxima ĉ(w):</b> %1<br>").arg(formatNumber(edge.maxCapacity));
    html += QString("<b>Rata r<sub>w</sub>:</b> %1<br>").arg(formatNumber(edge.rate));
    html += costDetails(edge);

    content_->setText(html);
}
