#ifndef PIECEITEM_H
#define PIECEITEM_H

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QPropertyAnimation>

class BoardScene;

class PieceItem : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_PROPERTY(QPointF scenePos READ scenePos WRITE setPos STORED false)

public:
    enum PixMapIndex {
        NORMAL,
        SELECTED
    };

    PieceItem(QChar ch, const QPointF& originPos, QGraphicsItem* parent = nullptr);

    int type() const override;
    static qreal diameter() { return 57; };
    static qreal halfDiameter() { return diameter() / 2; };

    QChar ch() const { return ch_; }
    QPointF originPos() const { return originPos_; }

    void leave();
    void setScenePos(const QPointF& pos);

    bool animation() const { return animation_; }
    void setAnimation(bool animation) { animation_ = animation; }

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;

private:
    QChar ch_;
    QPointF originPos_;
    QPointF oldPos;
    QPointF mousePos;

    QImage image_[2];
    int aniDuration_ { 600 };
    bool animation_ { false };

    QPropertyAnimation* propertyAnimation;
};

#endif // PIECEITEM_H
