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
    setRenderHint(QPainter::Antialiasing, true);
    setAutoFillBackground(true);

    parentItem = scene()->addRect(sceneRect());
}

int MoveView::getNodeItemNumPerPage() const
{
    return (height() - rootNodeItem->margin() * 2) / rootNodeItem->limitRect().height() - 1;
}

void MoveView::setNodeItemLayout(MoveNodeItemAlign align)
{
    rootNodeItem->layout(align);
}

void MoveView::setInstance(Instance* instance)
{
    ins = instance;
}

void MoveView::resetNodeItems()
{
    for (auto& item : parentItem->childItems())
        delete item;

    rootNodeItem = new MoveNodeItem(ins->getRootMove(), parentItem);
    QRectF rect = rootNodeItem->limitRect();
    scene()->setSceneRect(0, 0,
        (ins->maxCol() + 1) * rect.width() + margin_ * 2,
        (ins->maxRow() + 1) * rect.height() + margin_ * 2);

    rootNodeItem->genrateMoveNodeItem(parentItem);
    rootNodeItem->layout(MoveNodeItemAlign::LEFT);
}

void MoveView::updateNodeItemSelected()
{
    scene()->clearSelection();
    PMove move = ins->getCurMove();
    for (auto& item : parentItem->childItems()) {
        if (item->data(0).toInt() != MoveItemType::NODE)
            continue;

        if (static_cast<MoveNodeItem*>(item)->move() == move) {
            item->setSelected(true); // 产生重绘
            item->ensureVisible(QRectF(), margin_ + hspacing_, margin_ + vspacing_);
            return;
        }
    }
}

void MoveView::mousePressEvent(QMouseEvent* event)
{
    lastPos = event->pos();
    auto item = itemAt(event->pos());
    PMove move;
    if (item && item->data(0).toInt() == MoveItemType::NODE
        && (move = static_cast<MoveNodeItem*>(item)->move()) != ins->getCurMove())
        emit mousePressed(move);

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
            qreal coefficient = numDegrees.ry() > 0 ? 1.05 : 1 / 1.05;
            scale(coefficient, coefficient);
            event->accept();
        }
    } else
        QGraphicsView::wheelEvent(event);
}
