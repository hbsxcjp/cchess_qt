#include "moveitem.h"
#include "boardpieces.h"
#include "manual.h"
#include "manualmove.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"

#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QPainter>
#include <QStyleOption>

MoveNodeItem* MoveNodeItem::creatRootMoveNodeItem(Manual* manual, QGraphicsItem* parent)
{
    MoveNodeItem* rootNodeItem = new MoveNodeItem(Q_NULLPTR, manual->manualMove()->rootMove(), parent);
    rootNodeItem->createMoveNodeItem(parent);

    return rootNodeItem;
}

void MoveNodeItem::updateLayout(MoveNodeItemAlign align)
{
    layout(align);
    MoveLinkItem* lineItem;
    for (auto& item : parentItem()->childItems())
        if ((lineItem = qgraphicsitem_cast<MoveLinkItem*>(item)))
            lineItem->trackNode();
}

QRectF MoveNodeItem::limitRect()
{
    const int hspacing = 20, vspacing = 15;
    QRectF rect = outlineRect();
    return rect.adjusted(-hspacing / 2, -vspacing / 2, +hspacing / 2, +vspacing / 2);
}

QRectF MoveNodeItem::outlineRect()
{
    const int padding = 8;
    QFontMetrics metrics(QFont("黑体", 10));
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
    qreal width = isSelected ? 2 : 1.5;
    Qt::PenStyle style = isSelected ? Qt::DotLine : Qt::SolidLine;
    QPen pen(outlineColor, width, style, Qt::RoundCap);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(pen);
    painter->setBrush(isSelected ? QColor(0xe6, 0xff, 0xff) : backgroundColor);
    painter->drawPath(shape());

    painter->setPen(textColor);
    QFont font = painter->font();
    font.setBold(isSelected);
    painter->setFont(font);
    painter->drawText(outlineRect(), Qt::AlignCenter, text_);
}

QPainterPath MoveNodeItem::shape() const
{
    QRectF rect = outlineRect();
    QPainterPath path;
    path.addRoundedRect(rect, roundness(rect.width()), roundness((rect.height())));
    return path;
}

MoveNodeItem::MoveNodeItem(MoveNodeItem* preNodeItem, Move* move, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , move_(move)
    , preNodeItem_(preNodeItem)
    , nextNodeItem_(Q_NULLPTR)
    , otherNodeItem_(Q_NULLPTR)
{
    setFlags(QGraphicsItem::ItemIsMovable
        | QGraphicsItem::ItemIsSelectable
        | QGraphicsItem::ItemIsFocusable);

    text_ = move->zhStr();
    if (move->isRoot()) {
        textColor = QColor(Qt::black);
        outlineColor = QColor(Qt::darkBlue);
        backgroundColor = QColor(0xcc, 0xee, 0xff);
    } else {
        bool isRed = PieceBase::getColorFromZh(text_.back()) == PieceColor::RED;
        textColor = isRed ? QColor(0xff, 0x00, 0x00) : QColor(0x1e, 0x1e, 0x1a);
        outlineColor = textColor;
        backgroundColor = isRed ? QColor(0xff, 0xe2, 0xac) : QColor(0xfa, 0xd4, 0x84);
    }
}

void MoveNodeItem::createMoveNodeItem(QGraphicsItem* parent)
{
    if (move_->hasNext()) {
        nextNodeItem_ = new MoveNodeItem(this, move_->nextMove(), parent);
        new MoveLinkItem(this, nextNodeItem_, false, parent);

        nextNodeItem_->createMoveNodeItem(parent);
    }

    if (move_->hasOther()) {
        otherNodeItem_ = new MoveNodeItem(preNodeItem_, move_->otherMove(), parent);
        new MoveLinkItem(preNodeItem_, otherNodeItem_, false, parent);
        new MoveLinkItem(this, otherNodeItem_, true, parent);

        otherNodeItem_->createMoveNodeItem(parent);
    }
}

void MoveNodeItem::setAlignPos(MoveNodeItemAlign align)
{
    colIndexF_ = nextNodeItem_ ? nextNodeItem_->colIndexF() : move_->cc_ColIndex();
    MoveNodeItem* otherItem;
    if (nextNodeItem_ && (otherItem = nextNodeItem_->otherItem())) {
        while (otherItem->otherItem())
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

void MoveNodeItem::layout(MoveNodeItemAlign align)
{
    if (nextNodeItem_)
        nextNodeItem_->layout(align);

    if (otherNodeItem_)
        otherNodeItem_->layout(align);

    setAlignPos(align);
}

int MoveNodeItem::roundness(double size)
{
    const int diameter = 6;
    return 100 * diameter / int(size);
}

MoveLinkItem::MoveLinkItem(MoveNodeItem* fromNode, MoveNodeItem* toNode,
    bool isDashLine, QGraphicsItem* parent)
    : QGraphicsLineItem(parent)
    , fromNode_(fromNode)
    , toNode_(toNode)
{
    setZValue(-1);
    setPen(QPen(Qt::darkGray, isDashLine ? 1.0 : 2.0,
        isDashLine ? Qt::DashLine : Qt::SolidLine));
}

void MoveLinkItem::trackNode()
{
    setLine(QLineF(fromNode_->scenePos(), toNode_->scenePos()));
}
