#include "moveitem.h"
#include "move.h"
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
    setData(0, MoveItemType::NODE);
    setFlags(QGraphicsItem::ItemIsMovable
        | QGraphicsItem::ItemIsSelectable
        | QGraphicsItem::ItemSendsScenePositionChanges
        | QGraphicsItem::ItemIsFocusable);

    outlineColor = QColor(Qt::darkBlue);
    if (!move->preMove()) {
        text_ = QString("　开始　");
        textColor = QColor(Qt::black);
        backgroundColor = QColor("#fad484");
    } else {
        text_ = move->zhStr();
        bool isRed = Pieces::getColorFromZh(text_.back()) == PieceColor::RED;
        textColor = QColor(isRed ? "#ff3300" : "#1e1e1a");
        outlineColor = textColor;
        backgroundColor = QColor(isRed ? "#ffe2ac" : "#fad484");
    }
}

void MoveNodeItem::addLinkItem(MoveLinkItem* linkItem)
{
    linkItemList_.append(linkItem);
}

void MoveNodeItem::genrateMoveNodeItem(QGraphicsItem* parent)
{
    if (move_->nextMove()) {
        nextNodeItem_ = new MoveNodeItem(move_->nextMove(), parent);
        nextNodeItem_->setPreItem(this);
        new MoveLinkItem(this, nextNodeItem_, false, parent);

        nextNodeItem_->genrateMoveNodeItem(parent);
    }

    if (move_->otherMove()) {
        otherNodeItem_ = new MoveNodeItem(move_->otherMove(), parent);
        otherNodeItem_->setPreItem(preNodeItem_);
        new MoveLinkItem(preNodeItem_, otherNodeItem_, false, parent);
        new MoveLinkItem(this, otherNodeItem_, true, parent);

        otherNodeItem_->genrateMoveNodeItem(parent);
    }
}

void MoveNodeItem::setAlignPos(MoveNodeItemAlign align)
{
    bool hasNext { nextNodeItem_ };
    colIndexF_ = hasNext ? nextNodeItem_->colIndexF() : move_->cc_ColIndex();
    MoveNodeItem* otherItem;
    if (hasNext && (otherItem = nextNodeItem_->otherItem())) {
        while (otherItem && otherItem->otherItem())
            otherItem = otherItem->otherItem();

        if (align == MoveNodeItemAlign::CENTER)
            colIndexF_ = (colIndexF_ + otherItem->colIndexF()) / 2;
        else if (align == MoveNodeItemAlign::RIGHT)
            colIndexF_ = otherItem->colIndexF();
    }

    QRectF rect = limitRect();
    setPos(colIndexF_ * rect.width() + rect.width() / 2 + margin(),
        move_->nextIndex() * rect.height() + rect.height() / 2 + margin());
}

void MoveNodeItem::updateLinkItemPos()
{
    for (auto& linkItem : linkItemList_)
        linkItem->trackNode();
}

void MoveNodeItem::layout(MoveNodeItemAlign align)
{
    if (nextNodeItem_)
        nextNodeItem_->layout(align);

    if (otherNodeItem_)
        otherNodeItem_->layout(align);

    setAlignPos(align);
    updateLinkItemPos();
}

QRectF MoveNodeItem::limitRect()
{
    const int hspacing = 30, vspacing = 20;
    QRectF rect = outlineRect();
    return rect.adjusted(-hspacing / 2, -vspacing / 2, +hspacing / 2, +vspacing / 2);
}

QRectF MoveNodeItem::outlineRect()
{
    const int padding = 10;
    QFontMetrics metrics(QFont("黑体", 12));
    QRectF rect = metrics.boundingRect("马四进三"); // text_
    rect.adjust(-padding, -padding, +padding, +padding);
    rect.translate(-rect.center());
    return rect;
}

QRectF MoveNodeItem::boundingRect() const
{
    const int margin = 3;
    return outlineRect().adjusted(-margin, -margin, +margin, +margin);
}

void MoveNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    bool isSelected = option->state & QStyle::State_Selected;
    qreal width = isSelected ? 3 : 2;
    Qt::PenStyle style = isSelected ? Qt::DotLine : Qt::SolidLine;
    QPen pen(outlineColor, width, style, Qt::RoundCap);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(pen);
    painter->setBrush(isSelected ? QColor("#e6ffff") : backgroundColor);

    QRectF rect = outlineRect();
    painter->drawRoundedRect(rect, roundness(rect.width()), roundness(rect.height()));

    painter->setPen(textColor);
    QFont font = painter->font();
    font.setBold(isSelected);
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignCenter, text_);
}

QPainterPath MoveNodeItem::shape() const
{
    QRectF rect = outlineRect();
    QPainterPath path;
    path.addRoundedRect(rect, roundness(rect.width()), roundness((rect.height())));
    return path;
}

// QVariant MoveNodeItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
//{
//     if (change == ItemPositionHasChanged)
//         updateLinkItemPos();

//    return QGraphicsItem::itemChange(change, value);
//}

// void MoveNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
//{
//     Q_UNUSED(event);

//    setZValue(8);
//}

// void MoveNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
//{
//     setPos(event->scenePos() - event->pos());

//    updateLinkItemShow();
//}

// void MoveNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
//{
//     Q_UNUSED(event);

//    setZValue(0);
//}

// void MoveNodeItem::focusInEvent(QFocusEvent* event)
//{
//     Q_UNUSED(event)
//     if (isSelected())
//         return;

//    static int maxZValue = 1;
//    setZValue(maxZValue++);

//    emit curMoveChanged(move_);
//}

// void MoveNodeItem::focusOutEvent(QFocusEvent* event)
//{
//     Q_UNUSED(event)

//    //    setZValue(0);
//}

int MoveNodeItem::roundness(double size) const
{
    const int diameter = 12;
    return 100 * diameter / int(size);
}

MoveLinkItem::MoveLinkItem(MoveNodeItem* fromNode, MoveNodeItem* toNode,
    bool isDashLine, QGraphicsItem* parent)
    : QGraphicsLineItem(parent)
    , fromNode_(fromNode)
    , toNode_(toNode)
{
    setData(0, MoveItemType::LINE);
    setZValue(-1);
    setPen(QPen(Qt::darkGray, isDashLine ? 1.0 : 3.0,
        isDashLine ? Qt::DashLine : Qt::SolidLine));

    fromNode->addLinkItem(this);
    toNode->addLinkItem(this);
}

void MoveLinkItem::trackNode()
{
    setLine(QLineF(fromNode_->pos(), toNode_->pos()));
}
