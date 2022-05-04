#ifndef MOVEITEM_H
#define MOVEITEM_H

#include <QFont>
#include <QGraphicsItem>

class Move;
class ChessManual;

class MoveLinkItem;

enum class MoveNodeItemAlign {
    LEFT,
    CENTER,
    RIGHT
};

enum ItemType {
    MOVENODE = 1,
    MOVELINK,
};

class MoveNodeItem : public QGraphicsItem {

public:
    static MoveNodeItem* getRootMoveNodeItem(ChessManual* instance, QGraphicsItem* parent);

    enum { Type = UserType + ItemType::MOVENODE };
    int type() const override { return Type; }

    void updateLayout(MoveNodeItemAlign align);

    static int margin() { return 15; }
    static QRectF limitRect();
    static QRectF outlineRect();

    qreal colIndexF() const { return colIndexF_; }
    Move* move() const { return move_; }
    //    MoveNodeItem* nextItem() const { return nextNodeItem_; };
    MoveNodeItem* otherItem() const { return otherNodeItem_; };

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;

private:
    MoveNodeItem(MoveNodeItem* preNodeItem, Move* move, QGraphicsItem* parent);
    void genrateMoveNodeItem(QGraphicsItem* parent);

    void setAlignPos(MoveNodeItemAlign align);
    void layout(MoveNodeItemAlign align);

    static int roundness(double size);

    QString text_;
    QColor textColor;
    QColor backgroundColor;
    QColor outlineColor;
    qreal colIndexF_;

    Move* move_;

    MoveNodeItem* preNodeItem_;
    MoveNodeItem* nextNodeItem_;
    MoveNodeItem* otherNodeItem_;
};

class MoveLinkItem : public QGraphicsLineItem {
public:
    MoveLinkItem(MoveNodeItem* fromNode, MoveNodeItem* toNode,
        bool isDashLine, QGraphicsItem* parent);

    enum { Type = UserType + ItemType::MOVELINK };
    int type() const override { return Type; }

    void trackNode();

private:
    MoveNodeItem* fromNode_;
    MoveNodeItem* toNode_;
};

#endif // MOVEITEM_H
