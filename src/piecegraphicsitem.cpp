#include "piecegraphicsitem.h"
#include "boardgraphicsscene.h"
#include "instance.h"
#include "piece.h"
#include "seat.h"

static const qreal moveZValue { 8 };

PieceGraphicsItem::PieceGraphicsItem(QChar ch, const QPointF& originPos,
    BoardGraphicsScene* scene, QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
    , ch_(ch)
    , originPos_(originPos)
    , boardScene(scene)
    , propertyAnimation(new QPropertyAnimation(this, "scenePos", this))
{
    for (PixMapIndex index : { PixMapIndex::NORMAL, PixMapIndex::SELECTED }) {
        QString fileName { QString("%1/%2%3.GIF")
                               .arg(parentItem()->data(DataIndex::IMAGEDIR).toString())
                               .arg(ch == Pieces::nullChar
                                       ? "OOS"
                                       : QString("%1%2").arg(ch.isUpper() ? 'R' : 'B').arg(ch.toUpper()))
                               .arg(index == PixMapIndex::SELECTED ? "S" : "") };
        pixmap_[index].load(fileName);
    }

    setSelectedPixMap(PixMapIndex::NORMAL);
    propertyAnimation->setDuration(600);
    propertyAnimation->setEasingCurve(QEasingCurve::InOutCubic);
}

void PieceGraphicsItem::leave()
{
    setScenePos(originPos_);
}

void PieceGraphicsItem::setScenePos(const QPointF& pos)
{
    if (ch() != Pieces::nullChar && animation_) {
        propertyAnimation->setStartValue(scenePos());
        propertyAnimation->setEndValue(pos);
        propertyAnimation->start();
    } else
        setPos(pos); // 设置为父项坐标点（父项坐标已设置成与场景坐标相同）
}

void PieceGraphicsItem::setSelectedPixMap(PixMapIndex index)
{
    setPixmap(pixmap_[index]);
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
            setScenePos(boardScene->getSeatPos(toPos));
        } else
            // 终点不在棋盘
            setScenePos(boardScene->getLimitPos(toPos - mousePos));
    } else
        setScenePos(oldPos);

    setZValue(0);
}

void PieceGraphicsItem::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event)

    setSelectedPixMap(PixMapIndex::SELECTED);
    boardScene->showHintItem(boardScene->getCenterSeatPos(scenePos()), ch());
}

void PieceGraphicsItem::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event)

    setSelectedPixMap(PixMapIndex::NORMAL);
    boardScene->clearHintItem();
}
