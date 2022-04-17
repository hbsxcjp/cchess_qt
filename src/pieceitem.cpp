#include "pieceitem.h"
#include "boardview.h"
#include "instance.h"
#include "moveitem.h"
#include "piece.h"
#include "seat.h"
#include <QPainter>
#include <QStyleOption>

PieceItem::PieceItem(QChar ch, const QPointF& originPos, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , ch_(ch)
    , originPos_(originPos)
    , propertyAnimation(new QPropertyAnimation(this, "scenePos", this))
{
    for (PixMapIndex index : { PixMapIndex::NORMAL, PixMapIndex::SELECTED }) {
        QString fileName { QString("%1/%2%3.GIF")
                               .arg(parentItem()->data(0).toString())
                               .arg(ch == Pieces::nullChar
                                       ? "OO"
                                       : QString("%1%2").arg(ch.isUpper() ? 'R' : 'B').arg(ch.toUpper()))
                               .arg(index == PixMapIndex::SELECTED ? "S" : "") };
        image_[index] = QImage(fileName);
    }

    propertyAnimation->setDuration(aniDuration_);
    propertyAnimation->setEasingCurve(QEasingCurve::InOutCubic);
}

int PieceItem::type() const
{
    return UserType + ItemType::PIECE;
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

QRectF PieceItem::boundingRect() const
{
    return { 0, 0, diameter(), diameter() };
}

void PieceItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /* widget*/)
{
    bool isSelected = (ch_ == Pieces::nullChar) || (option->state & QStyle::State_Selected);
    painter->drawImage(boundingRect(), image_[isSelected]);
}

QPainterPath PieceItem::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}
