#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#define ERR_MESSAGE(title, content) \
    QMessageBox::critical(this, tr(title), tr(content), QMessageBox::Ok)
#define INF_MESSAGE(title, content) \
    QMessageBox::information(this, tr(title), tr(content), QMessageBox::Ok)

MainWindow::~MainWindow()
{
    close();
    delete _graph_drawer;
    delete _scene;
    delete _view;
    delete ui;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    /* variables and allocation */
    _view=new QGraphicsView(this);
    _scene=new QGraphicsScene(this);
    _graph_drawer=new GraphDrawer(_scene);
    /* parameters */
    ui->setupUi(this);
    _view->setScene(_scene);
    setCentralWidget(_view);
    /* connections */
    /* - ui components - */
    connect(ui->actionQuitter, &QAction::triggered,
            qApp, &QApplication::quit);
    connect(ui->actionOuvrir, &QAction::triggered,
            this, &MainWindow::open);
    connect(ui->actionSauvegarder, &QAction::triggered,
            this, &MainWindow::save);
    connect(ui->actionFermer, &QAction::triggered,
            this, &MainWindow::close);
    connect(ui->action_propos, &QAction::triggered,
            this, &MainWindow::about);
    connect(ui->action_propos_de_Qt, &QAction::triggered,
            qApp, &QApplication::aboutQt);
}

void MainWindow::open()
{
    QString filename;
    QFile file;
    QTextStream input;
    filename=QFileDialog::getOpenFileName(this, tr("Open graph file"),
                                          QString(), tr("Graph files (*.grp)"));
    if (filename.isNull()) {
        return;
    }
    file.setFileName(filename);
    if (!file.open(QFile::ReadOnly)) {
        ERR_MESSAGE("Open file error", "Can't open file.");
        return;
    }
    input.setDevice(&file);
    if (!_graph.parse(&input)) {
        ERR_MESSAGE("Parse graph error", "Failed to parse graph.");
        file.close();
        return;
    }
    file.close();
    _graph_drawer->draw(_graph);
    _graph_drawer->draw(_graph.entrelacs());
    INF_MESSAGE("Graph loaded", "Graph loaded!");
}

void MainWindow::save()
{
    QString filename;
    QFile file;
    QTextStream output;
    filename=QFileDialog::getSaveFileName(this, tr("Select save file"),
                                          QString(), tr("Graph files (*.grp)"));
    if (filename.isNull()) {
        return;
    }
    file.setFileName(filename);
    if (!file.open(QFile::WriteOnly|QFile::Truncate)) {
        ERR_MESSAGE("Open file error", "Can't open file.");
        return;
    }
    output.setDevice(&file);
    if (!_graph.save(&output)) {
        ERR_MESSAGE("Save graph error", "Failed to save graph.");
        file.close();
        return;
    }
    file.close();
    _graph_drawer->clear();
    INF_MESSAGE("Graph saved", "Graph saved!");
}

void MainWindow::close()
{
    _graph_drawer->clear();
    _graph.clear();
}

void MainWindow::about()
{

}

