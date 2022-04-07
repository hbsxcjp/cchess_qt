#include "movegraphicsscene.h"
#include "instance.h"
#include "movegraphicsitem.h"
#include <QPainter>

MoveGraphicsScene::MoveGraphicsScene(Instance* instance)
    : QGraphicsScene()
    , instance_(instance)
    , parentItem(addRect(sceneRect()))
    , rootNodeItem(Q_NULLPTR)
{
    setSceneRect(0, 0, width_, height_);
}

void MoveGraphicsScene::resetMoveNodeItem()
{
    for (auto& item : parentItem->childItems())
        delete item;

    rootNodeItem = new MoveNodeItem(instance_->getRootMove(), parentItem);
    rootNodeItem->addMoveNodeItem(parentItem);
    rootNodeItem->updateNodeItemShow();
}

void MoveGraphicsScene::setCurMoveSelected()
{
    PMove move = instance_->getCurMove();
    for (auto item : parentItem->childItems())
        item->setSelected(static_cast<MoveNodeItem*>(item)->moveIs(move));
}

void MoveGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::lightGray, 0, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    painter->drawRect(rect);
}
