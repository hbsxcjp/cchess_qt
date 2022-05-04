#include "pieceitem.h"
#include "boardpieces.h"
#include "boardview.h"
#include "instance.h"
#include "moveitem.h"
#include "piece.h"
#include "seat.h"
#include <QPainter>
#include <QStyleOption>

PieceItem::PieceItem(const QPointF& originPos, Piece* piece, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , originPos_(originPos)
    , piece_(piece)
    , view(static_cast<BoardView*>(parent->scene()->parent()))
    , propertyAnimation(new QPropertyAnimation(this, "scenePos", this))
{
    for (PixMapIndex index : { PixMapIndex::NORMAL, PixMapIndex::SELECTED }) {
        QChar ch = piece->ch();
        image_[index] = QImage(QString("%1/%2%3%4.GIF")
                                   .arg(parentItem()->data(0).toString())
                                   .arg(ch.isUpper() ? 'R' : 'B')
                                   .arg(ch.toUpper())
                                   .arg(index == PixMapIndex::SELECTED ? "S" : ""));
    }

    propertyAnimation->setDuration(aniDuration_);
    propertyAnimation->setEasingCurve(QEasingCurve::InOutCubic);
}

void PieceItem::leave()
{
    setScenePos(originPos_);
}

void PieceItem::setScenePos(const QPointF& pos)
{
    //    if (ch() != BoardPieces::nullChar && animation_) {
    if (animation_) {
        propertyAnimation->setStartValue(scenePos());
        propertyAnimation->setEndValue(pos);
        propertyAnimation->start();
    } else
        setPos(pos); // 设置为父项坐标点（父项坐标已设置成与场景坐标相同）
}

QRectF PieceItem::boundingRect() const
{
    return { 0, 0, diameter(), diameter() };
}

void PieceItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /* widget*/)
{
    bool isSelected = (option->state & QStyle::State_Selected);
    painter->drawImage(boundingRect(), image_[isSelected]);
}

QPainterPath PieceItem::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void PieceItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    oldPos = pos();
    mousePos = event->pos();

    setZValue(MOVEZVALUE);
    view->showHint(oldPos, this);

    QGraphicsItem::mousePressEvent(event);
}

void PieceItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    setPos(view->getLimitPos(event->scenePos() - mousePos));

    //        QGraphicsView::mouseMoveEvent(event);
}

void PieceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event)

    // 鼠标位置
    QPointF fromPos = oldPos + mousePos,
            toPos = event->scenePos();
    // 起点至终点可走
    if (view->canMovePos(fromPos, toPos, this)) {
        if (view->atBoard(toPos)) {
            // 终点在棋盘上
            setScenePos(view->getSeatPos(toPos));
        } else
            // 终点不在棋盘
            setScenePos(view->getLimitPos(toPos - mousePos));
    } else
        setScenePos(oldPos);

    setZValue(0);
    view->clearHintItem();
}
