#include "movegraphicsitem.h"
#include "move.h"
#include "movegraphicsscene.h"
#include "piece.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>

MoveNodeItem::MoveNodeItem(PMove move, MoveGraphicsScene* moveScene, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , move_(move)
    , moveScene_(moveScene)
    , text_(move->zhStr())
    , font_(QFont("隶书", 12))
    , textColor(QColor(Qt::red))
    , backgroundColor(QColor(Qt::yellow))
    , outlineColor(QColor(Qt::darkBlue))
    , nextNodeItem_(Q_NULLPTR)
    , otherNodeItem_(Q_NULLPTR)
{
    setFlags(QGraphicsItem::ItemIsMovable
        | QGraphicsItem::ItemIsSelectable
        | QGraphicsItem::ItemIsFocusable);

    // 根节点
    if (!move->preMove())
        text_ = QString("　开始　");
}

void MoveNodeItem::addMoveNodeItem(QGraphicsItem* parent)
{
    if (move_->nextMove()) {
        nextNodeItem_ = new MoveNodeItem(move_->nextMove(), moveScene_, parent);
        nextNodeItem_->addMoveNodeItem(parent);
        //        qDebug() << __LINE__ << move_->nextMove()->zhStr();
    }

    if (move_->otherMove()) {
        otherNodeItem_ = new MoveNodeItem(move_->otherMove(), moveScene_, parent);
        otherNodeItem_->addMoveNodeItem(parent);
        //        qDebug() << __LINE__ << move_->otherMove()->zhStr();
    }
}

void MoveNodeItem::updateNodeItemShow()
{
    const int margin = 30, hspacing = 40, vspacing = 50;
    QRectF rect = outlineRect();
    setPos(move_->cc_ColIndex() * (rect.width() + hspacing) + margin,
        move_->nextIndex() * (rect.height() + vspacing) + margin);

    //    qDebug() << __LINE__ << move_->zhStr();
    std::function<void(QPointF, QPointF, QGraphicsItem*)>
        setLineItem_ = [&](QPointF fromPos, QPointF toPos, QGraphicsItem* parent) {
            QGraphicsLineItem* lineItem = new QGraphicsLineItem(parent);
            lineItem->setZValue(-1);
            lineItem->setPen(QPen(Qt::darkGray, 1.0));
            lineItem->setLine(QLineF(fromPos, toPos));
        };

    if (nextNodeItem_) {
        setLineItem_(pos(), nextNodeItem_->pos(), parentItem());
        nextNodeItem_->updateNodeItemShow();
    }

    if (otherNodeItem_) {
        setLineItem_(pos(), otherNodeItem_->pos(), parentItem());
        otherNodeItem_->updateNodeItemShow();
    }
}

QRectF MoveNodeItem::boundingRect() const
{
    const int margin = 1;
    return outlineRect().adjusted(-margin, -margin, +margin, +margin);
}

void MoveNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    QPen pen(outlineColor);
    if (option->state & QStyle::State_Selected) {
        pen.setStyle(Qt::DotLine);
        pen.setWidth(2);
    }
    painter->setPen(pen);
    painter->setBrush(backgroundColor);

    QRectF rect = outlineRect();
    painter->drawRoundedRect(rect, roundness(rect.width()), roundness(rect.height()));

    painter->setPen(textColor);
    painter->drawText(rect, Qt::AlignCenter, text_);
}

QPainterPath MoveNodeItem::shape() const
{
    QRectF rect = outlineRect();
    QPainterPath path;
    path.addRoundedRect(rect, roundness(rect.width()), roundness((rect.height())));
    return path;
}

QRectF MoveNodeItem::outlineRect() const
{
    const int padding = 8;
    QFontMetrics metrics(font_);
    QRectF rect = metrics.boundingRect(text_);
    rect.adjust(-padding, -padding, +padding, +padding);
    rect.translate(-rect.center());
    return rect;
}

int MoveNodeItem::roundness(double size) const
{
    const int diameter = 12;
    return 100 * diameter / int(size);
}
