#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "graph.h"
#include "graphdrawer.h"

class QGraphicsView;    /* pre-decl */
class QGraphicsScene;   /* pre-decl */

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    QGraphicsView *_view;
    QGraphicsScene *_scene;
    GraphDrawer *_graph_drawer;
    Graph _graph;

public:
    virtual ~MainWindow();
    explicit MainWindow(QWidget *parent = 0);

private slots:
    /* ui slots */
    void open();
    void save();
    void close();
    void about();
};

#endif // MAINWINDOW_H
