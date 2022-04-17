#include "boardview.h"
#include "boardscene.h"
#include "common.h"
#include "instance.h"
#include "piece.h"
#include "pieceitem.h"
#include "seat.h"
#include <QDebug>
#include <QMouseEvent>
#include <QSettings>

#define NOTBOARDINDEX (-1)

static const qreal hintZValue { 4 };
static const qreal moveZValue { 8 };

BoardView::BoardView(QWidget* parent)
    : QGraphicsView(parent)
    , shadowItem(Q_NULLPTR)
    , hintParentItem(Q_NULLPTR)
    , pieceParentItem(Q_NULLPTR)
    , pieceItemList(QList<PieceItem*>())
{
    setScene(new BoardScene(QRectF(0, 0, leftWidth_ + boardWidth_, boardHeight_), this));
    scene()->setBackgroundBrush(QBrush(Qt::lightGray, Qt::SolidPattern));

    readSettings();
}

BoardView::~BoardView()
{
    writeSettings();
}

void BoardView::setInstance(Instance* instance)
{
    instance_ = instance;
    creatPieceItems();
}

bool BoardView::atBoard(const QPointF& pos) const
{
    return getBoardIndex(pos) != NOTBOARDINDEX;
}

void BoardView::allPieceToLeave()
{
    scene()->clearSelection();
    shadowItem->leave();
    for (auto item : pieceItemList)
        item->leave();
}

void BoardView::updatePieceItemShow()
{
    int curIndex = NOTBOARDINDEX;
    if (!instance_->isStartMove()) {
        SeatCoordPair seatCoordPair = instance_->getCurSeatCoordPair();
        shadowItem->setPos(getSeatPos(seatCoordPair.first));
        curIndex = Seats::getBoardIndex(seatCoordPair.second);
    } else
        shadowItem->leave();

    scene()->clearSelection();
    QString pieceChars = instance_->getPieceChars();
    QList<PieceItem*> copyPieceItemList(pieceItemList);
    for (PieceItem* item : pieceItemList) {
        int index = getBoardIndex(item->pos());
        if (index != NOTBOARDINDEX) {
            copyPieceItemList.removeOne(item);
            if (item->ch() == pieceChars.at(index)) {
                pieceChars[index] = Pieces::nullChar;
                if (curIndex == index) // 回退时
                    item->setSelected(true);
            } else
                copyPieceItemList.prepend(item);
        }
    }

    for (int index = 0; index < pieceChars.size(); ++index) {
        if (pieceChars.at(index) == Pieces::nullChar)
            continue;

        bool find { false };
        QMutableListIterator<PieceItem*> iterator(copyPieceItemList);
        while (iterator.hasNext()) {
            PieceItem* item = iterator.next();
            if (item->ch() == pieceChars.at(index)) {
                item->setScenePos(getSeatPos(index));
                if (curIndex == index) // 前进时
                    item->setSelected(true);

                iterator.remove();
                find = true;
                break;
            }
        }
        Q_ASSERT(find);
    }

    for (auto& item : copyPieceItemList)
        item->leave();
}

void BoardView::mousePressEvent(QMouseEvent* event)
{
    PieceItem* item;
    if (!(item = qgraphicsitem_cast<PieceItem*>(itemAt(event->pos()))))
        return;

    oldPos = item->pos();
    mousePos = event->globalPos();
    item->setZValue(moveZValue);

    QGraphicsView::mousePressEvent(event);
}

void BoardView::mouseMoveEvent(QMouseEvent* event)
{
    PieceItem* item;
    if (!(item = qgraphicsitem_cast<PieceItem*>(itemAt(event->pos()))))
        return;

    QPoint movePoint = event->globalPos() - mousePos;
    item->setPos(getLimitPos(oldPos + movePoint));

    //        QGraphicsView::mouseMoveEvent(event);
}

void BoardView::mouseReleaseEvent(QMouseEvent* event)
{
    PieceItem* item;
    if (!(item = qgraphicsitem_cast<PieceItem*>(itemAt(event->pos()))))
        return;

    QPointF toPos = item->pos();
    // 起点至终点可走
    if (canMovePos(oldPos, toPos, item->ch())) {
        if (atBoard(toPos)) {
            // 终点在棋盘上
            item->setScenePos(getSeatPos(toPos));
        } else
            // 终点不在棋盘
            item->setScenePos(getLimitPos(toPos));
    } else
        item->setScenePos(oldPos);

    item->setZValue(0);
}

// void PieceItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
//{
//     Q_UNUSED(event);

//}

// void PieceItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
//{
//     setPos(boardScene->getLimitPos(event->scenePos() - mousePos));
// }

// void PieceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
//{
//     QPointF fromPos = oldPos + mousePos,
//             toPos = event->scenePos();
//     // 起点至终点可走
//     if (boardScene->canMovePos(fromPos, toPos, ch())) {
//         if (boardScene->atBoard(toPos)) {
//             // 终点在棋盘上
//             setScenePos(boardScene->getSeatPos(toPos));
//         } else
//             // 终点不在棋盘
//             setScenePos(boardScene->getLimitPos(toPos - mousePos));
//     } else
//         setScenePos(oldPos);

//    setZValue(0);
//}

// void PieceItem::focusInEvent(QFocusEvent* event)
//{
//     Q_UNUSED(event)

//    setSelectedPixMap(PixMapIndex::SELECTED);
//    showHintItem(oldPos, item->ch());
//}

// void PieceItem::focusOutEvent(QFocusEvent* event)
//{
//     Q_UNUSED(event)

//    setSelectedPixMap(PixMapIndex::NORMAL);
//    boardScene->clearHintItem();
//}

bool BoardView::canMovePos(const QPointF& fromPos, const QPointF& toPos, QChar ch) const
{
    SeatCoord toSeatCoord = getSeatCoord(toPos);
    bool fromAtBoard { atBoard(fromPos) }, toAtBoard { atBoard(toPos) };
    switch (instance_->status()) {
    case InsStatus::LAYOUT:
        return toAtBoard ? instance_->canPut(ch).contains(toSeatCoord) : true;
    case InsStatus::PLAY:
        if (fromAtBoard && toAtBoard
            && instance_->canMove(getSeatCoord(fromPos)).contains(toSeatCoord))
            return true;

        return !fromAtBoard && !toAtBoard;
    case InsStatus::MOVEDEMO:
        break;
    default:
        break;
    }

    return false;
}

PieceItem* BoardView::getPieceItemAt(const QPointF& pos) const
{
    QPointF centerPos = pos + QPointF(PieceItem::halfDiameter(), PieceItem::halfDiameter());
    return qgraphicsitem_cast<PieceItem*>(itemAt(centerPos.rx(), centerPos.ry()));
}

void BoardView::showHintItem(const QPointF& scenePos, QChar ch)
{
    auto showHintItem_ = [&](QList<SeatCoord> seatCoordList) {
        qreal radius = PieceItem::diameter() * 2 / 3;
        QRectF rect(0, 0, radius, radius);
        QPen pen(instance_->status() == InsStatus::PLAY ? Qt::blue : Qt::darkGreen,
            instance_->status() == InsStatus::PLAY ? 3 : 2, Qt::DashLine, Qt::RoundCap);
        //    QBrush(Qt::lightGray, Qt::Dense6Pattern);
        hintParentItem = scene()->addRect(QRect(), Qt::NoPen);
        for (SeatCoord& seatCoord : seatCoordList) {
            QGraphicsEllipseItem* item = new QGraphicsEllipseItem(rect, hintParentItem);
            item->setPen(pen);
            item->setPos(getScenePos(seatCoord,
                posBoardStartX + PieceItem::diameter() / 6, PieceItem::diameter(),
                posStartY + PieceItem::diameter() / 6, PieceItem::diameter()));
            item->setZValue(hintZValue);
        }
    };

    switch (instance_->status()) {
    case InsStatus::LAYOUT:
        showHintItem_(instance_->canPut(ch));
        break;
    case InsStatus::PLAY:
        if (atBoard(scenePos))
            showHintItem_(instance_->canMove(getSeatCoord(scenePos)));
        break;
    case InsStatus::MOVEDEMO:
        break;
    default:
        break;
    }
}

void BoardView::clearHintItem()
{
    if (hintParentItem) {
        scene()->removeItem(hintParentItem);
        delete hintParentItem;
        hintParentItem = Q_NULLPTR;
    }
}

void BoardView::writeSettings() const
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::SCENEOPTION]);

    settings.setValue(stringLiterals[StringIndex::LEAVEPIECEORDER], leaveIsTidy);
    settings.setValue(stringLiterals[StringIndex::MOVEANIMATED], moveAnimated);
    settings.setValue(stringLiterals[StringIndex::BACKIMAGEFILE], backImageFile);
    settings.setValue(stringLiterals[StringIndex::PIECEIMAGEDIR], pieceImageDir);

    settings.endGroup();
}

void BoardView::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::SCENEOPTION]);

    leaveIsTidy = settings.value(stringLiterals[StringIndex::LEAVEPIECEORDER], true).toBool();
    moveAnimated = settings.value(stringLiterals[StringIndex::MOVEANIMATED], false).toBool();
    backImageFile = settings.value(stringLiterals[StringIndex::BACKIMAGEFILE], ":/res/IMAGES_L/WOOD.JPG").toString();
    pieceImageDir = settings.value(stringLiterals[StringIndex::PIECEIMAGEDIR], ":/res/IMAGES_L/WOOD").toString();

    settings.endGroup();
}

void BoardView::creatPieceItems()
{
    std::function<QPointF(PieceColor, int)>
        getOriginPos_ = [&](PieceColor color, int index) {
            int colNum = (leftWidth_ - PieceItem::diameter()) / PieceItem::halfDiameter();
            SeatCoord seatCoord(index / colNum, index % colNum);
            if (instance_->getHomeSide(color) == SeatSide::TOP)
                seatCoord = Seats::changeSeatCoord(seatCoord, ChangeType::SYMMETRY_V);
            return getScenePos(seatCoord, posStartX, PieceItem::halfDiameter(),
                posStartY, PieceItem::diameter());
        };

    pieceParentItem = scene()->addRect(QRect(), Qt::NoPen);
    pieceParentItem->setData(0, pieceImageDir);
    shadowItem = new PieceItem(Pieces::nullChar,
        { -PieceItem::diameter(), -PieceItem::diameter() }, pieceParentItem);
    QList<int> kindNum = { 1, 2, 2, 2, 2, 2, 5 };
    for (PieceColor color : { PieceColor::RED, PieceColor::BLACK }) {
        int kind = 0, index = 0;
        for (QChar ch : Pieces::chars[int(color)]) {
            for (int i = 0; i < kindNum.at(kind); ++i) {
                PieceItem* item = new PieceItem(ch, getOriginPos_(color, index),
                    pieceParentItem);
                item->setFlags(QGraphicsItem::ItemIsMovable
                    | QGraphicsItem::ItemIsSelectable
                    | QGraphicsItem::ItemIsFocusable);
                pieceItemList.append(item);

                index++;
            }
            kind++;
        }
    }

    allPieceToLeave();
}

QPointF BoardView::getLimitPos(const QPointF& pos) const
{
    bool atBoard = pos.x() > leftWidth_ - PieceItem::diameter() / 4;
    int startX = atBoard ? leftWidth_ : 0, width = atBoard ? boardWidth_ : leftWidth_,
        maxX = startX + width - PieceItem::diameter(), maxY = boardHeight_ - PieceItem::diameter();
    return QPoint(pos.x() < startX ? startX : (pos.x() > maxX ? maxX : pos.x()),
        pos.y() < 0 ? 0 : (pos.y() > maxY ? maxY : pos.y()));
}

QPointF BoardView::getSeatPos(int index) const
{
    return getSeatPos(Seats::getSeatCoord(index));
}

QPointF BoardView::getSeatPos(const QPointF& pos) const
{
    return getSeatPos(getSeatCoord(pos));
}

QPointF BoardView::getSeatPos(const SeatCoord& seatCoord) const
{
    return getScenePos(seatCoord, posBoardStartX, PieceItem::diameter(), posStartY, PieceItem::diameter());
}

int BoardView::getBoardIndex(const QPointF& pos) const
{
    if (pos.x() < leftWidth_)
        return NOTBOARDINDEX;

    return Seats::getBoardIndex(getSeatCoord(pos));
}

SeatCoord BoardView::getSeatCoord(const QPointF& pos) const
{
    int diameter = PieceItem::diameter(),
        col = int(pos.x() - leftWidth_) / diameter,
        row = int(pos.y()) / diameter;
    return Seats::changeSeatCoord({ row, col }, ChangeType::SYMMETRY_V);
}

QPointF BoardView::getScenePos(const SeatCoord& showCoord,
    qreal startX, qreal spacingX, qreal startY, qreal spacingY)
{
    SeatCoord seatCoord = Seats::changeSeatCoord(showCoord, ChangeType::SYMMETRY_V);
    return { startX + seatCoord.second * spacingX, startY + seatCoord.first * spacingY };
}
