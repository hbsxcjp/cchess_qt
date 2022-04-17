#include "moveview.h"
#include "instance.h"
#include "moveitem.h"
#include <QMouseEvent>
#include <QScrollBar>

MoveView::MoveView(QWidget* parent)
    : QGraphicsView(parent)
    , ins(Q_NULLPTR)
    , parentItem(Q_NULLPTR)
    , rootNodeItem(Q_NULLPTR)
{
    setScene(new QGraphicsScene(this));
    scene()->setBackgroundBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
}

void MoveView::setInstance(Instance* instance)
{
    ins = instance;
    parentItem = scene()->addRect(QRect(), Qt::NoPen);
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
    for (auto& item : parentItem->childItems())
        delete item;

    QRectF rect = MoveNodeItem::limitRect();
    scene()->setSceneRect(0, 0,
        (ins->maxCol() + 1) * rect.width() + margin_ * 2,
        (ins->maxRow() + 1) * rect.height() + margin_ * 2);

    rootNodeItem = MoveNodeItem::GetRootMoveNodeItem(ins, parentItem);
    rootNodeItem->updateLayout(MoveNodeItemAlign::LEFT);
}

void MoveView::updateNodeItemSelected()
{
    scene()->clearSelection();
    PMove move = ins->getCurMove();
    for (auto& aitem : parentItem->childItems()) {
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
    if (item && item->move() != ins->getCurMove())
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
