#include "piecegraphicsitem.h"
#include "boardgraphicsscene.h"
#include "instance.h"
#include "piece.h"
#include "seat.h"

static const qreal moveZValue { 8 };

PieceGraphicsItem::PieceGraphicsItem(BoardGraphicsScene* scene, QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
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

QPointF PieceGraphicsItem::leavePos() const
{
    return data(ItemDataIndex::LEAVEPOS).toPointF();
}

void PieceGraphicsItem::setLeavePos(const QPointF& leavePos)
{
    setData(ItemDataIndex::LEAVEPOS, leavePos);
}

void PieceGraphicsItem::leave()
{
    setBoardIndex(-1);
}

bool PieceGraphicsItem::atBoard() const
{
    return boardIndex() >= 0;
}

int PieceGraphicsItem::boardIndex() const
{
    return data(ItemDataIndex::BOARDINDEX).toInt();
}

void PieceGraphicsItem::setBoardIndex(int index)
{
    setData(ItemDataIndex::BOARDINDEX, index);
    if (atBoard())
        setPos(boardScene->getSeatPos(index));
    else {
        setPos(leavePos()); // 固定位置
        // 随机位置
    }
}

PieceColor PieceGraphicsItem::color() const
{
    return Pieces::getColor(ch());
}

void PieceGraphicsItem::setImageFile(bool selected)
{
    QString name { ch() == Pieces::nullChar
            ? "OOS"
            : QString("%1%2%3").arg(ch().isUpper() ? 'R' : 'B').arg(ch().toUpper()).arg(selected ? "S" : "") };
    setPixmap(QPixmap(QString("%1/%2.GIF")
                          .arg(parentItem()->data(ItemDataIndex::IMAGEFILETEMP).toString())
                          .arg(name)));
}

void PieceGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    oldPos = scenePos();
    mousePos = event->pos();
    setZValue(moveZValue);
}

void PieceGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    setPos(boardScene->getLimitPos(event->scenePos() - mousePos));
}

void PieceGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF fromPos = oldPos + mousePos,
            toPos = event->scenePos();
    // 起点至终点可走
    if (boardScene->canMovePos(fromPos, toPos, ch())) {
        if (boardScene->atBoard(toPos)) {
            // 终点在棋盘上
            setPos(boardScene->getSeatPos(toPos));
        } else
            // 终点不在棋盘
            setPos(boardScene->getLimitPos(toPos - mousePos));
    } else
        setPos(oldPos);

    setZValue(0);
}

void PieceGraphicsItem::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event)
    setImageFile(true);
    boardScene->showHintItem(boardScene->getCenterSeatPos(scenePos()), ch());
}

void PieceGraphicsItem::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event)
    setImageFile(false);
    boardScene->clearHintItem();
}
