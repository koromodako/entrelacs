#include "graph.h"

#include <QtMath>
#include <QVector2D>
#include <QTextStream>
#include <QDebug>

Graph::~Graph()
{
    clear();
}

Graph::Graph() :
    _nodes(),
    _arcs()
{

}

void Graph::clear()
{
    NodeHash::iterator nit;
    ArcHash::iterator ait;
    for (nit=_nodes.begin(); nit!=_nodes.end(); nit++) {
        delete nit.value();
    }
    for (ait=_arcs.begin(); ait!=_arcs.end(); ait++) {
        delete ait.value();
    }
    _nodes.clear();
    _arcs.clear();
}

bool Graph::parse(QTextStream *input)
{
    bool ret;
    QString line;
    uint id, id_src, id_dst;
    qreal x, y;
    QStringList data;
    QHash<uint, QUuid> ids;
    /* clear current data */
    clear();
    /* parse input stream */
    while ((*input).readLineInto(&line)) {
        line=line.trimmed();
        if (!line.startsWith('#')) {
            if (line.contains('=')) {
                /* process node decl */
                line.remove(0,1);
                data=line.split('=');
                if (data.size()!=2) { goto abrt; }
                id=data.first().toUInt(&ret);
                if (!ret) { goto abrt; }
                data=data.last().split(';');
                if (data.size()!=2) { ret=false; goto abrt; }
                x=data.first().right(data.first().length()-1).toDouble(&ret);
                if (!ret) { goto abrt; }
                y=data.last().left(data.last().length()-1).toDouble(&ret);
                if (!ret) { goto abrt; }
                ids.insert(id, add_node(QPointF(x, y)));
            } else {
                /* process arc decl*/
                data=line.split("->");
                if (data.size()!=2) { ret=false; goto abrt; }
                line=data.first().trimmed();
                id_src=line.right(line.length()-1).toUInt(&ret);
                if (!ret) { goto abrt; }
                line=data.last().trimmed();
                id_dst=line.right(line.length()-1).toUInt(&ret);
                if (!ret) { goto abrt; }
                connect(ids.value(id_src), ids.value(id_dst));
            }
        }
    }
    /* success */
    ret=true;
abrt:
    return ret;
}

bool Graph::save(QTextStream *output)
{
    uint ctr=0;
    QHash<QUuid, uint> ids;
    NodeHash::const_iterator it;
    (*output) << "# ---------- nodes ----------" << endl;
    for (it=_nodes.begin(); it!=_nodes.end(); ++it) {
        ids.insert(it.key(), ctr);
        (*output) << QString("n%0=[%1;%2]").arg(ctr)
                                        .arg(it.value()->position().x())
                                        .arg(it.value()->position().y())
               << endl;
        ctr++;
    }
    (*output) << "# ---------- arcs ----------" << endl;
    foreach (Arc *a, _arcs.values()) {
        (*output) << QString("n%0 -> n%1")
                     .arg(ids.value(a->const_src()->id()))
                     .arg(ids.value(a->const_dst()->id()))
                  << endl;
    }
    return true;
}

QUuid Graph::add_node(const QPointF &pos)
{
    Node *n=new Node(pos);
    _nodes.insert(n->id(), n);
    return n->id();
}

bool Graph::remove_node(const QPointF &pos)
{
    NodeHash::iterator nit, closest;
    qreal dist, min_dist=10e12;
    closest=_nodes.end();
    for (nit=_nodes.begin(); nit!=_nodes.end(); nit++) {
        if ((dist=distance(pos, nit.value()->position()))<min_dist) {
            min_dist=dist;
            closest=nit;
        }
    }
    if (closest!=_nodes.end()) {
        delete closest.value();
        return (_nodes.remove(closest.value()->id())!=0);
    }
    return false;
}

bool Graph::remove_node(const QUuid &nid)
{
    NodeHash::iterator nit=_nodes.find(nid);
    if (nit!=_nodes.end()) {
        delete nit.value();
    }
    return (_nodes.remove(nid)!=0);
}

const Node *Graph::node(const QUuid &nid) const
{
    const Node *n=nullptr;
    NodeHash::const_iterator nit=_nodes.constFind(nid);
    if (nit!=_nodes.constEnd()) {
        n=nit.value();
    }
    return n;
}

QList<const Node *> Graph::nodes() const
{
    QList<const Node*> ns;
    foreach (Node *n, _nodes.values()) {
        ns.append(n);
    }
    return ns;
}

QUuid Graph::connect(const QUuid &src_id, const QUuid &dst_id)
{
    NodeHash::iterator snit, dnit;
    Arc *arc;
    QUuid id;
    snit=_nodes.find(src_id);
    dnit=_nodes.find(dst_id);
    if (snit!=_nodes.end()&&dnit!=_nodes.end()) {
        arc=new Arc(snit.value(), dnit.value());
        snit.value()->attach(arc);
        dnit.value()->attach(arc);
        id=arc->id();
        _arcs.insert(id, arc);
    }
    return id;
}

QList<QUuid> Graph::connect(const QList<QUuid> &nodes)
{
    QList<QUuid>::const_iterator i, j;
    QList<QUuid> aids;
    i=nodes.begin();
    j=nodes.begin();
    j++;
    while (j!=nodes.end()) {
        aids.append(connect(*i, *j));
        i++, j++;
    }
    return aids;
}

bool Graph::disconnect(const QUuid &aid)
{
    ArcHash::iterator ait=_arcs.find(aid);
    if (ait!=_arcs.end()) {
        ait.value()->src()->detach(ait.value());
        ait.value()->dst()->detach(ait.value());
        delete ait.value();
        _arcs.erase(ait);
        return true;
    }
    return false;
}

void Graph::disconnect_incoming(const QUuid &nid)
{
    ArcHash::iterator ait;
    while (ait!=_arcs.end()) {
        if (ait.value()->dst()->id()==nid) {
            ait.value()->src()->detach(ait.value());
            ait.value()->dst()->detach(ait.value());
            delete ait.value();
            ait=_arcs.erase(ait);
        } else {
            ait++;
        }
    }
}

void Graph::disconnect_exiting(const QUuid &nid)
{
    ArcHash::iterator ait;
    while (ait!=_arcs.end()) {
        if (ait.value()->src()->id()==nid) {
            ait.value()->src()->detach(ait.value());
            ait.value()->dst()->detach(ait.value());
            delete ait.value();
            ait=_arcs.erase(ait);
        } else {
            ait++;
        }
    }
}

void Graph::disconnect_all(const QUuid &nid)
{
    ArcHash::iterator ait;
    while (ait!=_arcs.end()) {
        if (ait.value()->src()->id()==nid||ait.value()->dst()->id()==nid) {
            ait.value()->src()->detach(ait.value());
            ait.value()->dst()->detach(ait.value());
            delete ait.value();
            ait=_arcs.erase(ait);
        } else {
            ait++;
        }
    }
}

const Arc *Graph::arc(const QUuid &aid) const
{
    const Arc *a=nullptr;
    ArcHash::const_iterator ait=_arcs.constFind(aid);
    if (ait!=_arcs.constEnd()) {
        a=ait.value();
    }
    return a;
}

QList<const Arc *> Graph::arcs() const
{
    QList<const Arc*> as;
    foreach (Arc *a, _arcs.values()) {
        as.append(a);
    }
    return as;
}

QList<Entrelac> Graph::entrelacs() const
{
    return Entrelac::generate_from(*this);
}

qreal Graph::distance(const QPointF &src, const QPointF &dst)
{
    /* Manhattan distance */
    return qAbs(dst.x()-src.x())+qAbs(dst.y()-src.y());
}

Entrelac::~Entrelac()
{

}

Entrelac::Entrelac(const QPointF &start) :
    _start(start)
{

}

QList<Entrelac> Entrelac::generate_from(const Graph &graph)
{
    QList<Entrelac> entrelacs;
    PendingConnectionTable pending_table;
    /* initialize pending table */
    foreach (const Arc *a, graph.arcs()) {
        pending_table.insert(a, 4);
    }
    /* generate entrelacs */
    while (!pending_table.empty()) {
        entrelacs.append(generate_from(&pending_table));
    }
    return entrelacs;
}

#define DEBUG_POS(pos) "(" << pos.x() << "," << pos.y() << ")"
#define DEBUG_ARC(arc, name)                                                   \
    qDebug() << "D > > " << name << " arc is " << arc->id() << "("             \
             << DEBUG_POS(arc->const_src()->position())  << "->"               \
             << DEBUG_POS(arc->const_dst()->position()) << ")" << endl;

Entrelac Entrelac::generate_from(PendingConnectionTable *pending_table)
{
    qDebug() << "D > starting new entrelacs" << endl;
    const Arc *start_arc, *end_arc;
    const Node *start_node, *end_node;
    QPointF start_curve, end_curve;
    RotationDirection dir;
    qreal min_ang;
    PendingConnectionTable::iterator it;
    /* select first arc in pending table */
    start_arc=pending_table->begin().key();
    /* select vector direction depending on pending connection count */
    start_node=start_arc->const_src();
    end_node=start_arc->const_dst();
    if (pending_table->begin().value()==4) {
        dir=COUNTERCLOCKWISE_DIRECTION;
    } else {
        dir=CLOCKWISE_DIRECTION;
    }
    /* compute global start position */
    Entrelac entrelac(midpoint(start_node->position(), end_node->position()));
    qDebug() << "D > > entrelacs starts at " << DEBUG_POS(entrelac.start())
             << endl;
    /* loop while entrelac is not closed */
    while (!pending_table->empty()&&
           (start_curve==entrelac.start()||end_curve!=entrelac.start())) {
        /* find next arc */
        end_arc=next_arc(end_node, start_arc, dir, &min_ang);
        DEBUG_ARC(start_arc, "start_arc")
        DEBUG_ARC(end_arc, "end_arc")
        /* compute cubic curve point for start arc */
        start_curve=midpoint(start_node->position(), end_node->position());
        /* update start and end node for end arc and next iteration */
        start_node=end_node;
        end_node=(start_node==end_arc->const_src()?
                      end_arc->const_dst(): end_arc->const_src());
        /* compute cubic curve point for end arc */
        end_curve=midpoint(start_node->position(), end_node->position());
        /* add cubic curve to entrelacs */
        qDebug() << "D > > curve starts at " << DEBUG_POS(start_curve)
                 << "and ends at " << DEBUG_POS(end_curve) << endl;
        entrelac.add_subcurve(generate_curve(start_curve, end_curve,
                                             start_node->position(), dir, min_ang));
        /* update pending table */
        it=pending_table->find(start_arc);
        it.value()-=1;
        if (it.value()==0) {
            pending_table->erase(it);
        }
        it=pending_table->find(end_arc);
        it.value()-=1;
        if (it.value()==0) {
            pending_table->erase(it);
        }
        /* change arc */
        start_arc=end_arc;
        /* change direction */
        dir=(dir==CLOCKWISE_DIRECTION?
                 COUNTERCLOCKWISE_DIRECTION: CLOCKWISE_DIRECTION);
    }
    qDebug() << "D > ending current entrelac" << endl;
    return entrelac;
}

const Arc *Entrelac::next_arc(const Node *n, const Arc *a_ref,
                              RotationDirection dir, qreal *min_ang)
{
    const Arc *min_a;
    qreal ang, ang_ref, max_ang;
    bool augmented=false;
    (*min_ang)=2*M_PI;
    max_ang=0;
    if (n->arcs().size()>1) {
        /* compute angle between x axis and ref. vector */
        ang_ref=angle(n, a_ref);
        /* find closest vector (angular distance) */
        foreach (const Arc* a, n->arcs()) {
            if (a!=a_ref) {
                ang=angle(n, a);
                if (ang<ang_ref) {
                    ang+=2*M_PI;
                    augmented=true;
                }
                ang=ang-ang_ref;
                if (dir==COUNTERCLOCKWISE_DIRECTION&&ang<(*min_ang)) {
                    min_a=a;
                    if (augmented) {
                        ang=qAbs(ang-2*M_PI);
                    }
                    (*min_ang)=ang;
                } else if (dir==CLOCKWISE_DIRECTION&&ang>max_ang) {
                    min_a=a;
                    max_ang=ang;
                    (*min_ang)=ang;
                    if (augmented) {
                        (*min_ang)=ang_ref-((*min_ang)+ang_ref-2*M_PI);
                    } else {
                        (*min_ang)=2*M_PI-(*min_ang);
                    }
                }
            }
        }
    } else {
        min_a=n->arcs().first();
    }
    return min_a;
}

qreal Entrelac::angle(const Node *n, const Arc *a, qreal ref_angle)
{
    qreal ang, dx, dy, ref_cos, ref_sin;
    /* compute vector */
    if (a->const_src()==n) {
        dx=a->const_dst()->position().x()-n->position().x();
        dy=a->const_dst()->position().y()-n->position().y();
    } else {
        dx=a->const_src()->position().x()-n->position().x();
        dy=a->const_src()->position().y()-n->position().y();
    }
    /* apply rotation if necessary */
    if (ref_angle > 0.0) {
        ref_cos=qCos(ref_angle);
        ref_sin=qSin(ref_angle);
        dx=dx*ref_cos-dy*ref_sin;
        dy=dx*ref_sin+dy*ref_cos;
    }
    /* return angle */
    ang=qAtan2(dy,dx);
    if (ang<0) {
        ang=(2*M_PI)+ang;
    }
    return ang;
}

QPointF Entrelac::midpoint(const QPointF &src, const QPointF &dst)
{
    return QPointF((dst.x()+src.x())/2, (dst.y()+src.y())/2);
}

CubicCurve Entrelac::generate_curve(const QPointF &start_curve,
                                    const QPointF &end_curve,
                                    const QPointF &center,
                                    RotationDirection dir,
                                    qreal ang)
{
    QPointF scp, dcp;
    QVector2D dirv, pdirv, fdirv, sdirv;
    qreal rcos, rsin, len, scale;
    rcos=0;
    rsin=1;
    /*
    if (dir==CLOCKWISE_DIRECTION) {
        rsin=-rsin;
    }
    */
    scale=adaptive_scale(ang);
    /* compute control points coordinates  */
    /* compute scp */
    dirv.setX(start_curve.x()-center.x());
    dirv.setY(start_curve.y()-center.y());
    pdirv.setX(dirv.x()*rcos+dirv.y()*rsin);
    pdirv.setY(dirv.y()*rcos-dirv.x()*rsin);
    pdirv.normalize();
    pdirv*=dirv.length();
    fdirv.setX(start_curve.x()-center.x()+pdirv.x());
    fdirv.setY(start_curve.y()-center.y()+pdirv.y());
    len=fdirv.length()/2;
    fdirv.normalize();
    fdirv*=len;
    scp.setX(center.x()+fdirv.x());
    scp.setY(center.y()+fdirv.y());
    sdirv.setX(scp.x()-start_curve.x());
    sdirv.setY(scp.y()-start_curve.y());
    scp.setX(start_curve.x()+scale*sdirv.x());
    scp.setY(start_curve.y()+scale*sdirv.y());
    /* compute dcp */
    dirv.setX(center.x()-end_curve.x());
    dirv.setY(center.y()-end_curve.y());
    pdirv.setX(dirv.x()*rcos+dirv.y()*rsin);
    pdirv.setY(dirv.y()*rcos-dirv.x()*rsin);
    pdirv.normalize();
    pdirv*=dirv.length();
    fdirv.setX(end_curve.x()-center.x()+pdirv.x());
    fdirv.setY(end_curve.y()-center.y()+pdirv.y());
    len=fdirv.length()/2;
    fdirv.normalize();
    fdirv*=len;
    dcp.setX(center.x()+fdirv.x());
    dcp.setY(center.y()+fdirv.y());
    sdirv.setX(dcp.x()-end_curve.x());
    sdirv.setY(dcp.y()-end_curve.y());
    dcp.setX(end_curve.x()+scale*sdirv.x());
    dcp.setY(end_curve.y()+scale*sdirv.y());
    qDebug() << "D > > > generating curve with scp " << DEBUG_POS(scp)
             << " dcp " << DEBUG_POS(dcp) << endl;
    /* return cubic curve */
    return CubicCurve(scp, dcp, end_curve);
}

qreal Entrelac::adaptive_scale(qreal ang)
{
    qreal coeff=1;
    if (ang <= M_PI) {
        coeff=normalize(ang, 0, M_PI, 0, 1); // normalization [0,PI]->[0,1]
    } else {
        coeff*=normalize(ang, M_PI, 2*M_PI, 1, 3); //normalization [PI,2*PI]->[0,1]
    }
    return coeff;
}

qreal Entrelac::normalize(qreal x, qreal min_x, qreal max_x, qreal to_min_x, qreal to_max_x)
{
    return to_min_x + ((x-min_x) * (to_max_x-to_min_x)) / (max_x-min_x);
}
