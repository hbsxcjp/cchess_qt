#include "pieceitem.h"
#include "boardscene.h"
#include "instance.h"
#include "piece.h"
#include "seat.h"

static const qreal moveZValue { 8 };

PieceItem::PieceItem(QChar ch, const QPointF& originPos,
    BoardScene* scene, QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
    , ch_(ch)
    , originPos_(originPos)
    , boardScene(scene)
    , propertyAnimation(new QPropertyAnimation(this, "scenePos", this))
{
    for (PixMapIndex index : { PixMapIndex::NORMAL, PixMapIndex::SELECTED }) {
        QString fileName { QString("%1/%2%3.GIF")
                               //                               .arg(parentItem()->data(DataIndex::IMAGEDIR).toString())
                               .arg(":/res/IMAGES_L/WOOD")
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

void PieceItem::leave()
{
    setScenePos(originPos_);
}

void PieceItem::setScenePos(const QPointF& pos)
{
    if (ch() != Pieces::nullChar && animation_) {
        propertyAnimation->setStartValue(scenePos());
        propertyAnimation->setEndValue(pos);
        propertyAnimation->start();
    } else
        setPos(pos); // 设置为父项坐标点（父项坐标已设置成与场景坐标相同）
}

void PieceItem::setSelectedPixMap(PixMapIndex index)
{
    setPixmap(pixmap_[index]);
}

void PieceItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);

    oldPos = scenePos();
    mousePos = event->pos();
    setZValue(moveZValue);
}

void PieceItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    setPos(boardScene->getLimitPos(event->scenePos() - mousePos));
}

void PieceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
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

void PieceItem::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event)

    setSelectedPixMap(PixMapIndex::SELECTED);
    boardScene->showHintItem(boardScene->getCenterSeatPos(scenePos()), ch());
}

void PieceItem::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event)

    setSelectedPixMap(PixMapIndex::NORMAL);
    boardScene->clearHintItem();
}
