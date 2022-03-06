#include "piecegraphicsitem.h"
#include "boardgraphicsscene.h"
#include "piece.h"
#include "seat.h"

enum ItemDataIndex {
    CH,
};

static const qreal moveZValue { 8 };

PieceGraphicsItem::PieceGraphicsItem(BoardGraphicsScene* scene)
    : QGraphicsPixmapItem()
    , boardScene(scene)
{
}

QChar PieceGraphicsItem::ch() const
{
    return data(ItemDataIndex::CH).toChar();
}

void PieceGraphicsItem::setCh(QChar ch)
{
    setData(ItemDataIndex::CH, ch);
}

PieceColor PieceGraphicsItem::color() const
{
    return Pieces::getColor(ch());
}

void PieceGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
    setZValue(moveZValue);

    oldPos = scenePos();
    if (boardScene->getInsStatus() == InsStatus::INITLAYOUT)
        boardScene->setHintItemList(boardScene->getPutSeatCoordList(this));
    else {
    }
}

void PieceGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF pointf = event->scenePos();
    setPos(boardScene->atBoard(pointf)
            ? boardScene->getBoardPointF(pointf)
            : boardScene->getLeavePointF(pointf));
}

void PieceGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF pointf = event->scenePos();
    if (boardScene->atBoard(pointf)) {
        SeatCoord seatCoord = boardScene->getSeatCoord(pointf);
        QList<SeatCoord> putSeatCoordList = boardScene->getPutSeatCoordList(this);
        if (putSeatCoordList.contains(seatCoord)) {
            oldPos = boardScene->getSeatPointF(seatCoord);
        }
        setPos(oldPos);
    }

    setZValue(0);
    boardScene->clearHintItemList();
}
