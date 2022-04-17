#include "boardscene.h"
#include "boardview.h"
#include <QPainter>

void BoardScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect)

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::lightGray, 0, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    painter->drawRect(sceneRect());

    BoardView* view = static_cast<BoardView*>(parent());
    painter->drawImage(view->boardSceneRect(), QImage(view->getBackImageFile()));
}
