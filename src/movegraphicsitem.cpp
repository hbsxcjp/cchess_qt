#include "movegraphicsitem.h"
#include "move.h"
#include "movegraphicsscene.h"
#include "piece.h"

#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QPainter>
#include <QStyleOption>

MoveNodeItem::MoveNodeItem(PMove move, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , move_(move)
    , preNodeItem_(Q_NULLPTR)
    , nextNodeItem_(Q_NULLPTR)
    , otherNodeItem_(Q_NULLPTR)
    , linkItemList_(QList<MoveLinkItem*> {})
{
    setFlags(QGraphicsItem::ItemIsMovable
        | QGraphicsItem::ItemIsSelectable
        | QGraphicsItem::ItemSendsScenePositionChanges
        | QGraphicsItem::ItemIsFocusable);

    font_ = QFont("黑体", 12);
    backgroundColor = QColor(Qt::white);
    outlineColor = QColor(Qt::black);
    if (!move->preMove()) {
        text_ = QString("　开始　");
        textColor = QColor(Qt::black);
        backgroundColor = QColor(Qt::green);
    } else {
        text_ = move->zhStr();
        bool isRed = Pieces::getColorFromZh(text_.back()) == PieceColor::RED;
        textColor = QColor(isRed ? Qt::red : Qt::black);
        outlineColor = QColor(isRed ? Qt::darkBlue : Qt::blue);
    }
}

void MoveNodeItem::addLinkItem(MoveLinkItem* linkItem)
{
    linkItemList_.append(linkItem);
}

void MoveNodeItem::addMoveNodeItem(QGraphicsItem* parent)
{
    if (move_->nextMove()) {
        nextNodeItem_ = new MoveNodeItem(move_->nextMove(), parent);
        nextNodeItem_->setPreItem(this);
        new MoveLinkItem(this, nextNodeItem_, parent);

        nextNodeItem_->addMoveNodeItem(parent);
    }

    if (move_->otherMove()) {
        otherNodeItem_ = new MoveNodeItem(move_->otherMove(), parent);
        otherNodeItem_->setPreItem(preNodeItem_);
        new MoveLinkItem(preNodeItem_, otherNodeItem_, parent);

        otherNodeItem_->addMoveNodeItem(parent);
    }
}

void MoveNodeItem::setOriginPos()
{
    const int margin = 30, hspacing = 30, vspacing = 20;
    QRectF rect = outlineRect();
    setPos(move_->cc_ColIndex() * (rect.width() + hspacing) + rect.width() / 2 + margin,
        move_->nextIndex() * (rect.height() + vspacing) + margin);
}

void MoveNodeItem::updateLinkItemShow()
{
    for (auto& linkItem : linkItemList_)
        linkItem->trackNode();
}

void MoveNodeItem::updateNodeItemShow()
{
    setOriginPos();
    if (nextNodeItem_)
        nextNodeItem_->updateNodeItemShow();

    if (otherNodeItem_)
        otherNodeItem_->updateNodeItemShow();

    updateLinkItemShow();
}

QRectF MoveNodeItem::boundingRect() const
{
    const int margin = 3;
    return outlineRect().adjusted(-margin, -margin, +margin, +margin);
}

void MoveNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    QPen pen(outlineColor, 2, Qt::SolidLine, Qt::RoundCap);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(pen);
    painter->setBrush(backgroundColor);
    if (option->state & QStyle::State_Selected) {
        pen.setStyle(Qt::DotLine);
        pen.setWidth(3);
        painter->setBrush(QColor(Qt::yellow));
    }

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

QVariant MoveNodeItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged)
        updateLinkItemShow();

    return QGraphicsItem::itemChange(change, value);
}

// void MoveNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
//{
//     Q_UNUSED(event);
//     setZValue(8);
// }

// void MoveNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
//{
//     setPos(event->scenePos() - event->pos());

//    updateLinkItemShow();
//}

// void MoveNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
//{
//     Q_UNUSED(event);
//     setZValue(0);
// }

// void MoveNodeItem::focusInEvent(QFocusEvent* event)
//{
//     Q_UNUSED(event)

//    setSelected(true);
//}

// void MoveNodeItem::focusOutEvent(QFocusEvent* event)
//{
//     Q_UNUSED(event)

//    setSelected(false);
//}

QRectF MoveNodeItem::outlineRect() const
{
    const int padding = 10;
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

MoveLinkItem::MoveLinkItem(MoveNodeItem* fromNode, MoveNodeItem* toNode, QGraphicsItem* parent)
    : QGraphicsLineItem(parent)
    , fromNode_(fromNode)
    , toNode_(toNode)
{
    setZValue(-1);
    setPen(QPen(Qt::darkGray, 3.0));

    fromNode->addLinkItem(this);
    toNode->addLinkItem(this);
}

void MoveLinkItem::trackNode()
{
    setLine(QLineF(fromNode_->pos(), toNode_->pos()));
}
