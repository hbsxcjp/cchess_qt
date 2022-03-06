#ifndef PIECEGRAPHICSITEM_H
#define PIECEGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>

class BoardGraphicsScene;
enum class PieceColor;

class PieceGraphicsItem : public QGraphicsPixmapItem {
public:
    PieceGraphicsItem(BoardGraphicsScene* scene);

    QChar ch() const;
    void setCh(QChar ch);

    PieceColor color() const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QPointF oldPos;

    BoardGraphicsScene* boardScene;
};

#endif // PIECEGRAPHICSITEM_H
