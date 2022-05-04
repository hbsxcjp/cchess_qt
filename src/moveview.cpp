#include "moveview.h"
#include "chessmanual.h"
#include "moveitem.h"
#include <QMouseEvent>
#include <QScrollBar>

MoveView::MoveView(QWidget* parent)
    : QGraphicsView(parent)
    , manual(Q_NULLPTR)
    , rootNodeItem(Q_NULLPTR)
{
    setScene(new QGraphicsScene(this));
    scene()->setBackgroundBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    nodeParentItem = scene()->addRect(QRect(), Qt::NoPen);
}

void MoveView::setManual(ChessManual* manual)
{
    this->manual = manual;
}

void MoveView::setNodeItemLayout(MoveNodeItemAlign align)
{
    rootNodeItem->updateLayout(align);
}

int MoveView::getNodeItemNumPerPage() const
{
    return (height() - MoveNodeItem::margin() * 2) / MoveNodeItem::limitRect().height() - 1;
}

void MoveView::resetNodeItems()
{
    for (auto& item : nodeParentItem->childItems()) {
        scene()->removeItem(item);
        delete item;
    }

    QRectF rect = MoveNodeItem::limitRect();
    scene()->setSceneRect(0, 0,
        (manual->maxCol() + 1) * rect.width() + margin_ * 2,
        (manual->maxRow() + 1) * rect.height() + margin_ * 2);

    rootNodeItem = MoveNodeItem::getRootMoveNodeItem(manual, nodeParentItem);
    rootNodeItem->updateLayout(MoveNodeItemAlign::LEFT);
}

void MoveView::updateNodeItemSelected()
{
    scene()->clearSelection();
    Move* move = manual->getCurMove();
    for (auto& aitem : nodeParentItem->childItems()) {
        MoveNodeItem* item = qgraphicsitem_cast<MoveNodeItem*>(aitem);
        if (item && item->move() == move) {
            item->setSelected(true); // 产生重绘
            item->ensureVisible(QRectF(), margin_ + hspacing_, margin_ + vspacing_);
            return;
        }
    }
}

void MoveView::mousePressEvent(QMouseEvent* event)
{
    lastPos = event->pos();
    MoveNodeItem* item = qgraphicsitem_cast<MoveNodeItem*>(itemAt(event->pos()));
    if (item && item->move() != manual->getCurMove())
        emit mousePressed(item->move());

    //    QGraphicsView::mousePressEvent(event);
}

void MoveView::mouseMoveEvent(QMouseEvent* event)
{
    QPointF moveStep = event->pos() - lastPos; //计算移动坐标
    //通过滚动条调整展示的scene位置
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - moveStep.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() - moveStep.y());

    lastPos = event->pos();

    //    QGraphicsView::mouseMoveEvent(event);
}

void MoveView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() == Qt::KeyboardModifier::ControlModifier) {
        QPoint numDegrees = event->angleDelta() / 8;
        if (!numDegrees.isNull()) {
            emit wheelScrolled(numDegrees.ry() > 0);
            event->accept();
        }
    } else
        QGraphicsView::wheelEvent(event);
}
