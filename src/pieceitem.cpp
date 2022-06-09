#include "pieceitem.h"
#include "boardpieces.h"
#include "boardview.h"
#include "manual.h"
#include "moveitem.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"

#include <QPainter>
#include <QStyleOption>

#define DIAMETER 57

PieceItem::PieceItem(const QPointF& originPos, Piece* piece, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , originPos_(originPos)
    , piece_(piece)
    , view(qobject_cast<BoardView*>(parent->scene()->parent()))
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

qreal PieceItem::diameter()
{
    return DIAMETER / Tools::getScreenScale();
}

qreal PieceItem::halfDiameter()
{
    return diameter() / 2;
}

void PieceItem::leave()
{
    moveToPos(originPos_);
}

void PieceItem::moveToPos(const QPointF& pos)
{
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
    fromSeatPos = pos();
    mousePos = event->pos();

    setZValue(MOVEZVALUE);
    view->showHintItem(piece_, fromSeatPos);

    //    QGraphicsItem::mousePressEvent(event);
}

void PieceItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    setPos(view->getLocatePos(event->scenePos() - mousePos));
}

void PieceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    //    QPointF toSeatPos { fromSeatPos };
    //    if (abs(pos().rx() - fromSeatPos.rx()) > PieceItem::halfDiameter()
    //        || abs(pos().ry() - fromSeatPos.ry()) > PieceItem::halfDiameter())
    //        toSeatPos = view->getMovedPos(piece_, fromSeatPos, event->scenePos(), mousePos);

    //    moveToPos(toSeatPos);

    moveToPos(view->getMovedPos(piece_, fromSeatPos, event->scenePos(), mousePos));
    setZValue(INITZVALUE);
    view->clearHintItem();
}
