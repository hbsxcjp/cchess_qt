#ifndef PIECEGRAPHICSITEM_H
#define PIECEGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>

class BoardGraphicsScene;
enum class PieceColor;

using SeatCoord = QPair<int, int>;

enum ItemDataIndex {
    CH,
    LEAVEPOS,
    BOARDINDEX,
    IMAGEFILETEMP,
};

class PieceGraphicsItem : public QGraphicsPixmapItem {
public:
    PieceGraphicsItem(BoardGraphicsScene* scene, QGraphicsItem* parent = nullptr);

    QChar ch() const;
    void setCh(QChar ch);

    QPointF leavePos() const;
    void setLeavePos(const QPointF& leavePos);
    void leave();

    bool atBoard() const;
    int boardIndex() const;
    void setBoardIndex(int index);

    PieceColor color() const;

    void setImageFile(bool selected);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    QPointF oldPos, mousePos;
    BoardGraphicsScene* boardScene;
};

#endif // PIECEGRAPHICSITEM_H
