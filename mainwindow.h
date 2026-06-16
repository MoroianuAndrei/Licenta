#pragma once

#include <QMainWindow>
#include <QString>
#include <QStackedWidget>

#include <optional>
#include <vector>

#include "backend/core/graph.h"
#include "frontend/UiModels.h"
#include "frontend/BenchmarkWidget.h"
#include "frontend/TestWidget.h"

class ControlPanelWidget;
class EdgeDetailsWidget;
class GenerateGraphWidget;
class GraphWidget;
class ResultWidget;
class SidebarWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onLoadGraphClicked();
    void onRunClicked();
    void onAlgorithmChanged(const QString& algorithm);
    void onParameterChanged(double value);
    void onEdgeSelected(const VisualEdge& edge);
    void onGeneratedGraph(const QString& filePath);

private:
    void buildUi();
    void applyStyle();

    void resetResultState();
    void refreshViewState();
    void logSeparator();

    void loadGraphFromFile(const QString& fileName);

    void runMCP();
    void runAMCMCPEP(double z);
    void runAMCPEBCP(double W);
    void runParametricSearch(double W);

    QString defaultDataPath() const;
    std::vector<VisualEdge> loadVisualEdgesFromCSV(const QString& fileName) const;

    SidebarWidget* sidebar_ = nullptr;

    QStackedWidget* stackedWidget_ = nullptr;
    QWidget* loadRunPage_ = nullptr;
    GenerateGraphWidget* generateGraphWidget_ = nullptr;
    BenchmarkWidget* benchmarkWidget_ = nullptr;
    TestWidget* testWidget_ = nullptr;

    ControlPanelWidget* controls_ = nullptr;
    GraphWidget* graphWidget_ = nullptr;
    EdgeDetailsWidget* edgeDetails_ = nullptr;
    ResultWidget* resultWidget_ = nullptr;

    std::optional<Graph> currentGraph_;
    QString lastLoadedGraphFileName_;
    std::vector<VisualEdge> visualEdges_;
    AlgorithmViewState viewState_;
};


