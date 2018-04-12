#ifndef GRAPH_H
#define GRAPH_H

#include <QUuid>
#include <QHash>
#include <QPointF>
#include <QList>

class QTextStream;
class Arc; /* pre-decl */

class Node
{
private:
    QUuid _id;
    QPointF _pos;
    QList<const Arc*> _arcs;
public:
    virtual ~Node() {}
    Node(const QPointF &pos) :
        _id(QUuid::createUuid()),
        _pos(pos), _arcs()
    {}
    inline QUuid id() const
    { return _id; }
    inline QPointF position() const
    { return _pos; }

    inline QList<const Arc*> arcs() const
    { return _arcs; }

    bool attach(const Arc *arc)
    {
        if (!_arcs.contains(arc)) {
            _arcs.append(arc);
            return true;
        }
        return false;
    }
    bool detach(const Arc *arc)
    { return _arcs.removeOne(arc); }
};

class Arc
{
private:
    QUuid _id;
    Node *_src;
    Node *_dst;
public:
    virtual ~Arc() {}
    Arc(Node *src, Node *dst) :
        _id(QUuid::createUuid()),
        _src(src), _dst(dst)
    {}
    inline QUuid id() const
    { return _id; }
    inline Node *src()
    { return _src; }
    inline Node *dst()
    { return _dst; }
    inline const Node *const_src() const
    { return _src; }
    inline const Node *const_dst() const
    { return _dst; }
};

typedef QHash<QUuid, Node*> NodeHash;
typedef QHash<QUuid, Arc*> ArcHash;

class CubicCurve
{
private:
    QPointF _src_ctl_pt;
    QPointF _dst_ctl_pt;
    QPointF _dst_pt;
public:
    CubicCurve(const QPointF &scp, const QPointF &dcp, const QPointF &dp) :
        _src_ctl_pt(scp), _dst_ctl_pt(dcp), _dst_pt(dp)
    {}
    inline QPointF src_ctl_pt() const
    { return _src_ctl_pt; }
    inline QPointF dst_ctl_pt() const
    { return _dst_ctl_pt; }
    inline QPointF dst_pt() const
    { return _dst_pt; }
};

typedef QList<CubicCurve> CubicCurveList;

class Graph; /* pre-decl */

enum RotationDirection {
    CLOCKWISE_DIRECTION,
    COUNTERCLOCKWISE_DIRECTION
};

typedef QHash<const Arc*, ushort> PendingConnectionTable;

class Entrelac
{
private:
    QPointF _start;
    CubicCurveList _subcurves;
public:
    virtual ~Entrelac();
    Entrelac(const QPointF &start);

    inline QPointF start() const
    { return _start; }
    inline CubicCurveList subcurves() const
    { return _subcurves; }
    inline void add_subcurve(const CubicCurve &curve)
    { _subcurves.append(curve); }

    static QList<Entrelac> generate_from(const Graph &graph);

private:
    static Entrelac generate_from(PendingConnectionTable *pending_table);
    static const Arc *next_arc(const Node *n, const Arc *a_ref,
                               RotationDirection dir, qreal *min_ang);
    static qreal angle(const Node *n, const Arc *a, qreal ref_angle=0.0);
    static QPointF midpoint(const QPointF &src, const QPointF &dst);
    static CubicCurve generate_curve(const QPointF &start_curve,
                                     const QPointF &end_curve,
                                     const QPointF &center,
                                     RotationDirection dir, qreal ang);
    static qreal adaptive_scale(qreal ang);
    static qreal normalize(qreal x, qreal min_x, qreal max_x,
                           qreal to_min_x, qreal to_max_x);
};

class Graph
{
private:
    NodeHash _nodes;
    ArcHash _arcs;
public:
    virtual ~Graph();
    Graph();

    void clear();

    bool parse(QTextStream *input);
    bool save(QTextStream *output);

    QUuid add_node(const QPointF &pos);
    bool remove_node(const QPointF &pos);
    bool remove_node(const QUuid &nid);

    const Node *node(const QUuid &nid) const;
    QList<const Node*> nodes() const;

    QUuid connect(const QUuid &src_id, const QUuid &dst_id);
    QList<QUuid> connect(const QList<QUuid> &nodes);
    bool disconnect(const QUuid &aid);
    void disconnect_incoming(const QUuid &nid);
    void disconnect_exiting(const QUuid &nid);
    void disconnect_all(const QUuid &nid);

    const Arc *arc(const QUuid &aid) const;
    QList<const Arc*> arcs() const;

    QList<Entrelac> entrelacs() const;

private:
    qreal distance(const QPointF &src, const QPointF &dst);
};

#endif // GRAPH_H
