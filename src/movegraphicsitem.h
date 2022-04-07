#ifndef MOVEGRAPHICSITEM_H
#define MOVEGRAPHICSITEM_H

#include <QFont>
#include <QGraphicsItem>

class Move;
using PMove = Move*;
class MoveGraphicsScene;

class MoveLinkItem;

class MoveNodeItem : public QGraphicsItem {
public:
    MoveNodeItem(PMove move, QGraphicsItem* parent);

    void addLinkItem(MoveLinkItem* linkItem);
    void addMoveNodeItem(QGraphicsItem* parent);
    void setOriginPos();

    void updateLinkItemShow();
    void updateNodeItemShow();

    bool moveIs(PMove move) const { return move == move_; }

    void setPreItem(MoveNodeItem* preNodeItem) { preNodeItem_ = preNodeItem; };
    MoveNodeItem* preItem() const { return preNodeItem_; };

    MoveNodeItem* nextItem() const { return nextNodeItem_; };
    MoveNodeItem* otherItem() const { return otherNodeItem_; };

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    //    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    //    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    //    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    //    void focusInEvent(QFocusEvent* event) override;
    //    void focusOutEvent(QFocusEvent* event) override;

private:
    QRectF outlineRect() const;
    int roundness(double size) const;

    QString text_;
    QFont font_;
    QColor textColor;
    QColor backgroundColor;
    QColor outlineColor;

    PMove move_;

    MoveNodeItem* preNodeItem_;
    MoveNodeItem* nextNodeItem_;
    MoveNodeItem* otherNodeItem_;

    QList<MoveLinkItem*> linkItemList_;
};

class MoveLinkItem : public QGraphicsLineItem {
public:
    MoveLinkItem(MoveNodeItem* fromNode, MoveNodeItem* toNode, QGraphicsItem* parent);

    void trackNode();

    bool moveIs(PMove /*move*/) const { return false; }

private:
    MoveNodeItem* fromNode_;
    MoveNodeItem* toNode_;
};

#endif // MOVEGRAPHICSITEM_H
