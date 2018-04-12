#include "graphdrawer.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QBrush>

#define NODE_WIDTH 10

GraphDrawer::~GraphDrawer()
{

}

GraphDrawer::GraphDrawer(QGraphicsScene *scene) :
    _scene(scene)
{

}

void GraphDrawer::clear()
{
    _scene->clear();
}

void GraphDrawer::draw(const Graph &graph)
{
    QGraphicsEllipseItem *eitm;
    QGraphicsLineItem *litm;
    /* draw nodes */
    foreach (const Node *n, graph.nodes()) {
        eitm=new QGraphicsEllipseItem(n->position().x()-(NODE_WIDTH/2),
                                      n->position().y()-(NODE_WIDTH/2),
                                      NODE_WIDTH, NODE_WIDTH);
        eitm->setPen(QPen(QBrush(Qt::black), 1.));
        eitm->setBrush(QBrush(Qt::black));
        _scene->addItem(eitm);
    }
    /* draw arcs */
    foreach (const Arc *a, graph.arcs()) {
        litm=new QGraphicsLineItem(a->const_src()->position().x(),
                                   a->const_src()->position().y(),
                                   a->const_dst()->position().x(),
                                   a->const_dst()->position().y());
        litm->setPen(QPen(QBrush(Qt::black), 1.));
        _scene->addItem(litm);
    }
}

void GraphDrawer::draw(const QList<Entrelac> &entrelacs)
{
    QPainterPath path;
    QGraphicsPathItem *pitm;
    foreach (const Entrelac e, entrelacs) {
        path=QPainterPath(e.start());
        foreach (const CubicCurve cc, e.subcurves()) {
            path.cubicTo(cc.src_ctl_pt(), cc.dst_ctl_pt(), cc.dst_pt());
        }
        pitm=new QGraphicsPathItem(path);
        pitm->setPen(QPen(QBrush(Qt::red), 1.));
        pitm->setBrush(QBrush(Qt::transparent));
        _scene->addItem(pitm);
    }
}
