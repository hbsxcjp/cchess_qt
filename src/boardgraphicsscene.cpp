#include "boardgraphicsscene.h"
#include <QGraphicsPixmapItem>
#include <QPainter>

BoardGraphicsScene::BoardGraphicsScene(int leftWidth, int boardWidth, int boardHeight)
    : QGraphicsScene()
    , leftWidth_(leftWidth)
    , boardWidth_(boardWidth)
    , boardHeight_(boardHeight)
{
    setSceneRect(0, 0, leftWidth + boardWidth, boardHeight);
    addPieceGraphicsItems();
}

void BoardGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect)

    QRect boardRect(leftWidth_, 0, boardWidth_, boardHeight_);
    painter->drawImage(boardRect, QImage(":/res/IMAGES_L/WOOD.JPG"));

    //    painter->setRenderHint(QPainter::Antialiasing, true);
    //        painter->setPen(QPen(Qt::darkGray, 1, Qt::SolidLine, Qt::RoundCap));
    //    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    //        painter->drawRect(boardSide_, boardSide_, leftWidth_ - boardSide_ * 2, boardHeight_);
    //    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    //    painter->drawRect(0, 0, leftWidth_, boardHeight_ + boardSide_ * 2);
    //    int vcenter = boardHeight_ / 2 + boardSide_,
    //        vupcenter = vcenter - 20, vdowncenter = vcenter + 20;
    //    painter->drawLine(QLineF(boardSide_, vupcenter, leftWidth_ - boardSide_, vupcenter));
    //    painter->drawLine(QLineF(boardSide_, vdowncenter, leftWidth_ - boardSide_, vdowncenter));
}

void BoardGraphicsScene::addPieceGraphicsItems()
{

    //    painter.drawImage(QRect(-9, -10, 2, 2), QImage(":/res/IMAGES_L/WOOD/BR.GIF"));
    //    painter.drawImage(QRect(7, 8, 2, 2), QImage(":/res/IMAGES_L/WOOD/RR.GIF"));

    QGraphicsItem* item = new QGraphicsPixmapItem(QPixmap(":/res/IMAGES_L/WOOD/RR.GIF"));
    item->setFlags(QGraphicsItem::ItemIsMovable
        | QGraphicsItem::ItemIsSelectable
        | QGraphicsItem::ItemIsFocusable);
    item->setPos(10, 10);

    addItem(item);
}
