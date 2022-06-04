#ifndef PIECEITEM_H
#define PIECEITEM_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QPropertyAnimation>

#define INITZVALUE 0
#define HINTZVALUE 16
#define MOVEZVALUE 8

class Piece;
class BoardView;

class PieceItem : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_PROPERTY(QPointF scenePos READ scenePos WRITE setPos STORED false)
    Q_INTERFACES(QGraphicsItem)

public:
    enum PixMapIndex {
        NORMAL,
        SELECTED
    };

    PieceItem(const QPointF& originPos, Piece* piece, QGraphicsItem* parent = nullptr);

    enum { Type = UserType + 3 };
    int type() const override { return Type; }

    static qreal diameter() { return 57; };
    static qreal halfDiameter() { return diameter() / 2; };

    Piece* piece() const { return piece_; }
    QPointF originPos() const { return originPos_; }

    void leave();
    void moveToPos(const QPointF& pos);

    bool animation() const { return animation_; }
    void setAnimation(bool animation) { animation_ = animation; }

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QPointF originPos_;
    QPointF fromSeatPos;
    QPointF mousePos;

    QImage image_[2];
    int aniDuration_ { 600 };
    bool animation_ { false };

    Piece* piece_;
    BoardView* view;
    QPropertyAnimation* propertyAnimation;
};

#endif // PIECEITEM_H
