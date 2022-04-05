#ifndef PIECEGRAPHICSITEM_H
#define PIECEGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QPropertyAnimation>

class BoardGraphicsScene;

enum DataIndex {
    IMAGEDIR
};

enum PixMapIndex {
    NORMAL,
    SELECTED
};

class PieceGraphicsItem : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(QPointF scenePos READ scenePos WRITE setPos STORED false)

public:
    PieceGraphicsItem(QChar ch, const QPointF& originPos,
        BoardGraphicsScene* scene, QGraphicsItem* parent = nullptr);

    QChar ch() const { return ch_; }
    QPointF originPos() const { return originPos_; }

    void leave();
    void setScenePos(const QPointF& pos);
    void setSelectedPixMap(PixMapIndex index);

    bool animation() const { return animation_; }
    void setAnimation(bool animation) { animation_ = animation; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    QChar ch_;
    QPointF originPos_;
    QPointF oldPos;
    QPointF mousePos;

    QPixmap pixmap_[2];
    bool animation_ { true };

    BoardGraphicsScene* boardScene;
    QPropertyAnimation* propertyAnimation;
};

#endif // PIECEGRAPHICSITEM_H
