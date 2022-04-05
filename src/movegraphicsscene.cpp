#include "movegraphicsscene.h"
#include "instance.h"
#include "movegraphicsitem.h"

MoveGraphicsScene::MoveGraphicsScene(Instance* instance)
    : QGraphicsScene()
    , instance_(instance)
    , parentItem(addRect(sceneRect()))
    , rootNodeItem(Q_NULLPTR)
{
    setSceneRect(0, 0, width_, height_);
}

void MoveGraphicsScene::updateMoveItemShow()
{
    for (auto& item : parentItem->childItems())
        delete item;

    rootNodeItem = new MoveNodeItem(instance_->getRootMove(), this, parentItem);
    rootNodeItem->addMoveNodeItem(parentItem);
    rootNodeItem->updateNodeItemShow();
}
