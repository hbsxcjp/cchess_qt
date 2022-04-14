#ifndef MOVEITEM_H
#define MOVEITEM_H

#include <QFont>
#include <QGraphicsItem>

class Move;
using PMove = Move*;

class MoveLinkItem;

enum MoveItemType {
    NODE,
    LINE
};

enum class MoveNodeItemAlign {
    LEFT,
    CENTER,
    RIGHT
};

class MoveNodeItem : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    MoveNodeItem(PMove move, QGraphicsItem* parent);

    void addLinkItem(MoveLinkItem* linkItem);
    void addMoveNodeItem(QGraphicsItem* parent);
    void genrateMoveNodeItem(QGraphicsItem* parent);

    void setAlignPos(MoveNodeItemAlign align);
    void updateLinkItemPos();
    void layout(MoveNodeItemAlign align);

    static int margin() { return 15; }
    static QRectF limitRect();
    static QRectF outlineRect();

    qreal colIndexF() const { return colIndexF_; }
    PMove move() const { return move_; }

    void setPreItem(MoveNodeItem* preNodeItem) { preNodeItem_ = preNodeItem; };
    MoveNodeItem* preItem() const { return preNodeItem_; };

    MoveNodeItem* nextItem() const { return nextNodeItem_; };
    MoveNodeItem* otherItem() const { return otherNodeItem_; };

signals:
    void curMoveChanged(PMove move);

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;

    //    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    //    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    //    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    //    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    //    void focusInEvent(QFocusEvent* event) override;
    //    void focusOutEvent(QFocusEvent* event) override;

private:
    int roundness(double size) const;

    QString text_;
    QColor textColor;
    QColor backgroundColor;
    QColor outlineColor;
    qreal colIndexF_;

    PMove move_;

    MoveNodeItem* preNodeItem_;
    MoveNodeItem* nextNodeItem_;
    MoveNodeItem* otherNodeItem_;

    QList<MoveLinkItem*> linkItemList_;
};

class MoveLinkItem : public QGraphicsLineItem {
public:
    MoveLinkItem(MoveNodeItem* fromNode, MoveNodeItem* toNode,
        bool isDashLine, QGraphicsItem* parent);

    void trackNode();

private:
    MoveNodeItem* fromNode_;
    MoveNodeItem* toNode_;
};

#endif // MOVEITEM_H
