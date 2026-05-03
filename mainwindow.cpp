#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GraphLoader.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPointF>
#include <QString>
#include <QColor>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "backend/algorithms/AMCMCPEP.h"
#include "backend/algorithms/AMCPEBCP.h"
#include "backend/algorithms/ParametricSearch.h"

namespace
{
    QString g_lastLoadedGraphFileName;

    struct VisualEdge
    {
        int from = 0;
        int to = 0;
        double capacity = 0.0;
        double maxCapacity = 0.0;
        bool isNew = false;
        double rate = 0.0;
    };

    std::vector<VisualEdge> g_visualEdges;
    std::vector<int> g_lastPath;
    bool g_hasAlgorithmResult = false;
    double g_lastResultZ = 0.0;

    QGraphicsScene* g_graphScene = nullptr;
    QGraphicsView*  g_graphView = nullptr;

    void resetGraphResult()
    {
        g_hasAlgorithmResult = false;
        g_lastPath.clear();
        g_lastResultZ = 0.0;
    }

    std::vector<VisualEdge> loadVisualEdgesFromCSV(const QString& fileName)
    {
        std::ifstream file(fileName.toStdString());
        if (!file.is_open())
            throw std::runtime_error("Nu pot deschide fisierul CSV pentru desenarea grafului.");

        std::vector<VisualEdge> edges;
        std::string line;
        std::getline(file, line); // header

        while (std::getline(file, line))
        {
            if (line.empty())
                continue;

            std::stringstream ss(line);
            VisualEdge e;
            int isNewInt = 0;
            char comma = ',';

            ss >> e.from >> comma
               >> e.to >> comma
               >> e.capacity >> comma
               >> e.maxCapacity >> comma
               >> isNewInt >> comma
               >> e.rate;

            if (!ss)
                throw std::runtime_error("Linie CSV invalida pentru desenare: " + line);

            e.isNew = (isNewInt != 0);
            edges.push_back(e);
        }

        return edges;
    }

    bool edgeIsInPath(const VisualEdge& e, const std::vector<int>& path)
    {
        for (size_t i = 0; i + 1 < path.size(); ++i)
        {
            if (path[i] == e.from && path[i + 1] == e.to)
                return true;
        }
        return false;
    }

    QString formatNumber(double value)
    {
        if (std::abs(value - std::round(value)) < 1e-6)
            return QString::number((int)std::round(value));
        return QString::number(value, 'f', 2);
    }

    QString buildEdgeLabel(const VisualEdge& e, const QString& selectedAlgo, double inputValue, bool afterRun)
    {
        QString label = QString("c=%1, ĉ=%2")
            .arg(formatNumber(e.capacity))
            .arg(formatNumber(e.maxCapacity));

        if (selectedAlgo == "AMCMCPEP")
        {
            const double z = afterRun ? g_lastResultZ : inputValue;
            const double distance = std::abs(e.to - e.from);
            double b = 0.0;
            double u = 0.0;

            if (!e.isNew)
            {
                const double diff = std::max(0.0, z - e.capacity);
                b = 10.0 * distance * diff;
                u = distance * diff * diff;
            }
            else
            {
                b = 4.0 * distance * z;
                const double diff = z - 10.0;
                u = distance * diff * diff;
            }

            label += QString("\nb=%1, u=%2")
                .arg(formatNumber(b))
                .arg(formatNumber(u));
        }
        else if (selectedAlgo == "Parametric Search")
        {
            label += QString("\nr=%1")
                .arg(formatNumber(e.rate));
        }

        if (e.isNew)
            label += "\nnew";

        return label;
    }

    QPainterPath makeArrowHead(const QPointF& start, const QPointF& end)
    {
        const double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
        const double arrowSize = 12.0;

        QPointF p1 = end - QPointF(std::cos(angle - M_PI / 6.0) * arrowSize,
                                   std::sin(angle - M_PI / 6.0) * arrowSize);
        QPointF p2 = end - QPointF(std::cos(angle + M_PI / 6.0) * arrowSize,
                                   std::sin(angle + M_PI / 6.0) * arrowSize);

        QPainterPath path;
        path.moveTo(end);
        path.lineTo(p1);
        path.lineTo(p2);
        path.closeSubpath();
        return path;
    }

    void drawGraph(const QString& selectedAlgo, double inputValue)
    {
        if (!g_graphScene)
            return;

        g_graphScene->clear();

        if (g_visualEdges.empty())
        {
            g_graphScene->addText("Incarca un fisier CSV pentru a desena graful.");
            return;
        }

        std::set<int> nodes;
        for (const auto& e : g_visualEdges)
        {
            nodes.insert(e.from);
            nodes.insert(e.to);
        }

        const int n = (int)nodes.size();
        const double radius = 190.0;
        const QPointF center(300.0, 240.0);
        std::map<int, QPointF> pos;

        int idx = 0;
        for (int node : nodes)
        {
            const double angle = -M_PI / 2.0 + 2.0 * M_PI * idx / std::max(1, n);
            pos[node] = QPointF(center.x() + radius * std::cos(angle),
                                center.y() + radius * std::sin(angle));
            ++idx;
        }

        const double nodeRadius = 20.0;

        for (const auto& e : g_visualEdges)
        {
            const bool used = g_hasAlgorithmResult && edgeIsInPath(e, g_lastPath);
            const bool unavailableAfterRun = g_hasAlgorithmResult && !used && e.maxCapacity + 1e-9 < g_lastResultZ;

            if (unavailableAfterRun)
                continue;

            QPointF a = pos[e.from];
            QPointF b = pos[e.to];
            QPointF dir = b - a;
            const double len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
            if (len <= 1e-9)
                continue;

            dir /= len;
            QPointF start = a + dir * nodeRadius;
            QPointF end = b - dir * nodeRadius;

            QPen pen;
            if (used)
            {
                pen = QPen(e.isNew ? QColor(0, 145, 90) : QColor(30, 110, 230), 4.0);
            }
            else
            {
                pen = QPen(e.isNew ? QColor(120, 120, 120) : QColor(70, 70, 70), 2.0);
            }

            if (e.isNew)
                pen.setStyle(Qt::DashLine);

            g_graphScene->addLine(QLineF(start, end), pen);

            QGraphicsPathItem* arrow = g_graphScene->addPath(makeArrowHead(start, end), pen, QBrush(pen.color()));
            arrow->setZValue(1);

            QPointF mid = (start + end) / 2.0;
            QPointF normal(-dir.y(), dir.x());
            QPointF labelPos = mid + normal * 24.0;

            QGraphicsTextItem* label = g_graphScene->addText(buildEdgeLabel(e, selectedAlgo, inputValue, g_hasAlgorithmResult));
            label->setDefaultTextColor(used ? pen.color() : QColor(230, 230, 230));
            label->setFont(QFont("Arial", 8));
            label->setPos(labelPos);
            label->setZValue(2);
        }

        for (int node : nodes)
        {
            QPointF p = pos[node];
            QRectF rect(p.x() - nodeRadius, p.y() - nodeRadius, nodeRadius * 2, nodeRadius * 2);

            QGraphicsEllipseItem* circle = g_graphScene->addEllipse(
                rect,
                QPen(QColor(30, 30, 30), 2.0),
                QBrush(QColor(245, 245, 245))
            );
            circle->setZValue(3);

            QGraphicsTextItem* text = g_graphScene->addText(QString::number(node));
            text->setDefaultTextColor(Qt::black);
            text->setFont(QFont("Arial", 10, QFont::Bold));
            QRectF br = text->boundingRect();
            text->setPos(p.x() - br.width() / 2.0, p.y() - br.height() / 2.0);
            text->setZValue(4);
        }

        QString title;
        if (g_hasAlgorithmResult)
            title = QString("Rezultat: z*=%1 | albastru=arc existent ales | verde punctat=arc nou ales")
                        .arg(formatNumber(g_lastResultZ));
        else
            title = "Graful incarcat: arce continue = existente, arce punctate = potential noi";

        QGraphicsTextItem* titleItem = g_graphScene->addText(title);
        titleItem->setDefaultTextColor(QColor(240, 240, 240));
        titleItem->setFont(QFont("Arial", 10, QFont::Bold));
        titleItem->setPos(15, 10);

        g_graphScene->setSceneRect(g_graphScene->itemsBoundingRect().adjusted(-40, -40, 40, 40));
        if (g_graphView)
            g_graphView->fitInView(g_graphScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    algoComboBox = new QComboBox(this);
    algoComboBox->addItem("AMCMCPEP");
    algoComboBox->addItem("AMCPEBCP");
    algoComboBox->addItem("Parametric Search");

    zSpinBox = new QDoubleSpinBox(this);
    zSpinBox->setRange(0, 1000000);
    zSpinBox->setDecimals(0);
    zSpinBox->setPrefix("z = ");

    loadGraphButton = new QPushButton("Load Graph", this);
    runButton = new QPushButton("Run Algorithm", this);

    controlsLayout->addWidget(algoComboBox);
    controlsLayout->addWidget(zSpinBox);
    controlsLayout->addWidget(loadGraphButton);
    controlsLayout->addWidget(runButton);

    outputText = new QTextEdit(this);
    outputText->setReadOnly(true);

    g_graphScene = new QGraphicsScene(this);
    g_graphView = new QGraphicsView(g_graphScene, this);
    g_graphView->setRenderHint(QPainter::Antialiasing);
    g_graphView->setMinimumHeight(420);
    g_graphView->setBackgroundBrush(QBrush(QColor(35, 35, 35)));

    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(g_graphView);
    splitter->addWidget(outputText);
    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 1);

    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(splitter);

    auto updateParameterPrefix = [this]() {
        const QString selectedAlgo = algoComboBox->currentText();

        if (selectedAlgo == "AMCMCPEP") {
            zSpinBox->setPrefix("z = ");
            zSpinBox->setToolTip("Desired MCP capacity z for Algorithm 1");
        } else {
            zSpinBox->setPrefix("W = ");
            zSpinBox->setToolTip("Expansion budget W for Algorithm 3 / Algorithm 4");
        }

        resetGraphResult();
        drawGraph(selectedAlgo, zSpinBox->value());
    };

    connect(algoComboBox, &QComboBox::currentTextChanged, this, updateParameterPrefix);

    connect(zSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        // Daca parametrul se modifica dupa rulare, solutia veche nu mai este valida.
        // Revenim la vizualizarea initiala a grafului incarcat si recalculam etichetele.
        resetGraphResult();
        drawGraph(algoComboBox->currentText(), value);
    });

    connect(loadGraphButton, &QPushButton::clicked, this, &MainWindow::onLoadGraphClicked);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::onRunClicked);

    updateParameterPrefix();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLoadGraphClicked()
{
    QString defaultPath = QCoreApplication::applicationDirPath();
    defaultPath += "/../../../data";

    static QString lastPath;
    QString startPath = lastPath.isEmpty() ? defaultPath : lastPath;

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Graph File",
        startPath,
        "CSV Files (*.csv);;All Files (*.*)"
    );

    if (fileName.isEmpty())
        return;

    lastPath = QFileInfo(fileName).absolutePath();

    try {
        g_lastLoadedGraphFileName = fileName;
        currentGraph = loadGraphFromCSV(fileName.toStdString(), CostModel::ArticleGeneral);
        g_visualEdges = loadVisualEdgesFromCSV(fileName);
        resetGraphResult();

        outputText->append("Loaded Graph from: " + fileName);
        outputText->append("Graph loaded successfully.");
        drawGraph(algoComboBox->currentText(), zSpinBox->value());
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::onRunClicked()
{
     if (!currentGraph) {
         QMessageBox::warning(this, "Warning", "Please load a graph first!");
         return;
     }

    QString selectedAlgo = algoComboBox->currentText();
    double inputValue = zSpinBox->value();
    const QString parameterName = (selectedAlgo == "AMCMCPEP") ? "z" : "W";

    outputText->append(QString("--- Running %1 with %2=%3 ---")
        .arg(selectedAlgo)
        .arg(parameterName)
        .arg(inputValue));

    try {
        if (selectedAlgo == "AMCMCPEP") {
            AMCMCPEP solver;
            auto result = solver.solve(*currentGraph, inputValue);

            outputText->append(QString("Feasible: %1")
                .arg(result.isFeasible ? "Yes" : "No"));

            outputText->append(QString("Total Cost: %1")
                .arg(result.totalCost));

            QString pathStr = "Path: ";
            if (result.path.empty()) {
                pathStr += "(no path found)";
            } else {
                for (size_t i = 0; i < result.path.size(); ++i) {
                    pathStr += QString::number(result.path[i]);
                    if (i + 1 < result.path.size())
                        pathStr += " -> ";
                }
            }
            outputText->append(pathStr);

            g_hasAlgorithmResult = result.isFeasible;
            g_lastResultZ = inputValue;
            g_lastPath = result.path;
            drawGraph(selectedAlgo, inputValue);
        } else if (selectedAlgo == "AMCPEBCP") {
            AMCPEBCP solver;
            auto result = solver.solve(*currentGraph, inputValue);

            outputText->append(QString("Budget W: %1").arg(inputValue));
            outputText->append(QString("Best z*: %1").arg(result.maxCapacity));
            outputText->append(QString("Final Cost: %1").arg(result.finalCost));

            QString pathStr = "Path: ";
            if (result.path.empty()) {
                pathStr += "(no path found)";
            } else {
                for (size_t i = 0; i < result.path.size(); ++i) {
                    pathStr += QString::number(result.path[i]);
                    if (i + 1 < result.path.size())
                        pathStr += " -> ";
                }
            }
            outputText->append(pathStr);

            g_hasAlgorithmResult = !result.path.empty();
            g_lastResultZ = result.maxCapacity;
            g_lastPath = result.path;
            drawGraph(selectedAlgo, inputValue);
        } else if (selectedAlgo == "Parametric Search") {
            if (g_lastLoadedGraphFileName.isEmpty())
                throw std::runtime_error("Nu exista un fisier CSV incarcat pentru reconstruirea grafului liniar.");

            Graph linearGraph = loadGraphFromCSV(g_lastLoadedGraphFileName.toStdString(), CostModel::Linear);

            ParametricSearch solver;
            auto result = solver.solve(linearGraph, inputValue);

            outputText->append(QString("Budget W: %1").arg(inputValue));
            outputText->append(QString("Best z*: %1").arg(result.maxCapacity));
            outputText->append(QString("Final Cost: %1").arg(result.finalCost));

            QString pathStr = "Path: ";
            if (result.path.empty()) {
                pathStr += "(no path found)";
            } else {
                for (size_t i = 0; i < result.path.size(); ++i) {
                    pathStr += QString::number(result.path[i]);
                    if (i + 1 < result.path.size())
                        pathStr += " -> ";
                }
            }
            outputText->append(pathStr);

            g_hasAlgorithmResult = !result.path.empty();
            g_lastResultZ = result.maxCapacity;
            g_lastPath = result.path;
            drawGraph(selectedAlgo, inputValue);
        }
    }
    catch(const std::exception& e) {
        outputText->append(QString("Error executing algorithm: %1").arg(e.what()));
    }

    outputText->append("-------------------");
    outputText->ensureCursorVisible();
}
