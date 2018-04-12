#ifndef GRAPHDRAWER_H
#define GRAPHDRAWER_H

#include <QGraphicsScene>
#include "graph.h"

class GraphDrawer
{
private:
    QGraphicsScene *_scene;
public:
    virtual ~GraphDrawer();
    GraphDrawer(QGraphicsScene *scene);

    void clear();

    void draw(const Graph &graph);
    void draw(const QList<Entrelac> &entrelacs);
};

#endif // GRAPHDRAWER_H
