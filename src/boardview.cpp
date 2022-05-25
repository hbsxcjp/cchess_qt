#include "boardview.h"
#include "boardpieces.h"
#include "boardscene.h"
#include "common.h"
#include "manual.h"
#include "manualmove.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "pieceitem.h"
#include "seat.h"
#include "seatbase.h"

#include <QDebug>
#include <QMouseEvent>
#include <QSettings>

#define TOPMARGIN 28
#define BOTTOMMARGIN 28
#define LEFTWIDTH 200
#define BOARDWIDTH 521
#define BOARDHEIGHT 577
#define SCENESTARTX 3
#define BOARDSTARTX (LEFTWIDTH + SCENESTARTX)
#define SCENESTARTY (TOPMARGIN + 3)
#define TEXTMARGINSTARTY 0
#define TEXTMARGINSTARTX 20

#define NOTBOARDINDEX (-1)
#define HINTZVALUE 4
#define OUTSIZE (-1000)

static QRect getBoardRect()
{
    return QRect(LEFTWIDTH, TOPMARGIN, BOARDWIDTH, BOARDHEIGHT);
}

static QRectF getSceneRect()
{
    return QRectF(0, 0, LEFTWIDTH + BOARDWIDTH, TOPMARGIN + BOARDHEIGHT + BOTTOMMARGIN);
}

BoardView::BoardView(QWidget* parent)
    : QGraphicsView(parent)
    , hintParentItem(Q_NULLPTR)
{
    //    setScene(new BoardScene(QRectF(0, 0, LEFTWIDTH + BOARDWIDTH, BOARDHEIGHT), this));
    setScene(new BoardScene(getSceneRect(), this));
    scene()->setBackgroundBrush(QBrush(Qt::lightGray, Qt::SolidPattern));

    pieceParentItem = scene()->addRect(QRect(), Qt::NoPen);

    readSettings();
    shadowItem = new QGraphicsPixmapItem(QPixmap(QString("%1/OOS.GIF").arg(pieceImageDir)));
    scene()->addItem(shadowItem);
}

BoardView::~BoardView()
{
    writeSettings();
}

void BoardView::setManual(Manual* manual)
{
    manual_ = manual;
    creatMarginItems();
    creatPieceItems();
}

QRect BoardView::boardRect() const
{
    //    return QRect(LEFTWIDTH, 0, BOARDWIDTH, BOARDHEIGHT);
    return getBoardRect();
}

bool BoardView::atBoard(const QPointF& pos) const
{
    return pos.x() > LEFTWIDTH;
}

void BoardView::allPieceToLeave()
{
    scene()->clearSelection();
    shadowItem->setPos(QPointF(OUTSIZE, OUTSIZE));
    for (auto& item : getPieceItems())
        item->leave();
}

// void BoardView::mousePressEvent(QMouseEvent* event)
//{
//     PieceItem* item;
//     if (!(item = qgraphicsitem_cast<PieceItem*>(itemAt(event->pos()))))
//         return;

//    oldPos = item->pos();
//    mousePos = event->globalPos();
//    scene()->clearSelection();
//    item->setSelected(true);

//    item->setZValue(MOVEZVALUE);
//    showHintItem(oldPos, item->ch());
//    //    QGraphicsView::mousePressEvent(event);
//}

// void BoardView::mouseMoveEvent(QMouseEvent* event)
//{
//     PieceItem* item;
//     if (!(item = qgraphicsitem_cast<PieceItem*>(itemAt(event->pos()))))
//         return;

//    QPoint movePoint = event->globalPos() - mousePos;
//    item->setPos(getLimitPos(oldPos + movePoint));

//    //        QGraphicsView::mouseMoveEvent(event);
//}

// void BoardView::mouseReleaseEvent(QMouseEvent* event)
//{
//     PieceItem* item;
//     if (!(item = qgraphicsitem_cast<PieceItem*>(itemAt(event->pos()))))
//         return;

//    QPointF toPos = item->pos();
//    // 起点至终点可走
//    if (canMovePos(oldPos, toPos, item->ch())) {
//        if (atBoard(toPos)) {
//            // 终点在棋盘上
//            item->setScenePos(getSeatPos(toPos));
//        } else
//            // 终点不在棋盘
//            item->setScenePos(getLimitPos(toPos));
//    } else
//        item->setScenePos(oldPos);

//    item->setZValue(0);
//    clearHintItem();
//}

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

QPointF BoardView::getLimitPos(const QPointF& pos) const
{
    bool atBoard = pos.x() > LEFTWIDTH - PieceItem::diameter() / 4;
    int startX = atBoard ? LEFTWIDTH : 0, width = atBoard ? BOARDWIDTH : LEFTWIDTH,
        maxX = startX + width - PieceItem::diameter(), maxY = BOARDHEIGHT - PieceItem::diameter();
    return QPoint(pos.x() < startX ? startX : (pos.x() > maxX ? maxX : pos.x()),
        pos.y() < 0 ? 0 : (pos.y() > maxY ? maxY : pos.y()));
}

QPointF BoardView::getSeatPos(const QPointF& pos) const
{
    return getSeatPos(getCoord(pos));
}

bool BoardView::canMovePos(const QPointF& fromPos, const QPointF& toPos, const PieceItem* item) const
{
    Coord toSeatCoord = getCoord(toPos);
    bool fromAtBoard { atBoard(fromPos) }, toAtBoard { atBoard(toPos) };
    switch (manual_->status()) {
    case ManualStatus::LAYOUT:
        return toAtBoard ? manual_->canPut(item->piece()).contains(toSeatCoord) : true;
    case ManualStatus::PLAY:
        if (fromAtBoard && toAtBoard
            && manual_->canMove(getCoord(fromPos)).contains(toSeatCoord))
            return true;

        return !fromAtBoard && !toAtBoard;
    case ManualStatus::MOVEDEMO:
        break;
    default:
        break;
    }

    return false;
}

void BoardView::showHint(const QPointF& scenePos, PieceItem* pieceItem)
{
    hintParentItem = scene()->addRect(QRect(), Qt::NoPen);
    auto showHint_ = [&](const QList<Coord>& coords) {
        qreal radius = PieceItem::diameter() * 2 / 3;
        qreal startX { BOARDSTARTX + PieceItem::diameter() / 6 };
        qreal startY { SCENESTARTY + PieceItem::diameter() / 6 };
        qreal spacing { PieceItem::diameter() };
        QRectF rect(0, 0, radius, radius);
        QPen pen(manual_->status() == ManualStatus::PLAY ? Qt::blue : Qt::darkGreen,
            manual_->status() == ManualStatus::PLAY ? 3 : 2, Qt::DashLine, Qt::RoundCap);
        //    QBrush(Qt::lightGray, Qt::Dense6Pattern);
        for (const Coord& coord : coords) {
            QGraphicsEllipseItem* item = new QGraphicsEllipseItem(rect, hintParentItem);
            item->setPen(pen);
            item->setPos(getScenePos(coord, startX, spacing, startY, spacing));
            item->setZValue(HINTZVALUE);
        }
    };

    switch (manual_->status()) {
    case ManualStatus::LAYOUT:
        showHint_(manual_->canPut(pieceItem->piece()));
        break;
    case ManualStatus::PLAY:
        if (atBoard(scenePos))
            showHint_(manual_->canMove(getCoord(scenePos)));
        break;
    case ManualStatus::MOVEDEMO:
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
    }
}

void BoardView::updatePieceItemShow()
{
    Seat* curSeat {};
    if (!manual_->manualMove()->move()->isRoot()) {
        SeatPair seatPair = manual_->manualMove()->getCurSeatPair();
        shadowItem->setPos(getSeatPos(seatPair.first->coord()));
        curSeat = seatPair.second;
    } else
        shadowItem->setPos(QPointF(OUTSIZE, OUTSIZE));

    scene()->clearSelection();
    QList<Seat*> liveSeats { manual_->getLiveSeats() };
    QList<PieceItem*> copyItemList { getPieceItems() };
    QMutableListIterator<Seat*> seatIterator(liveSeats);
    while (seatIterator.hasNext()) {
        Seat* seat = seatIterator.next();
        Coord coord = seat->coord();

        QMutableListIterator<PieceItem*> itemIterator(copyItemList);
        while (itemIterator.hasNext()) {
            PieceItem* item = itemIterator.next();
            if (seat->piece() == item->piece()) {
                if (coord != getCoord(item->pos()))
                    item->setScenePos(getSeatPos(coord));
                if (curSeat == seat)
                    item->setSelected(true);

                itemIterator.remove();
                break;
            }
        }
    }

    for (auto& item : copyItemList)
        item->leave();
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

void BoardView::creatMarginItems()
{
    QFont font = this->font();
    font.setBold(true);
    for (auto color : PieceBase::ALLCOLORS) {
        bool isTop { manual_->getHomeSide(color) == SeatSide::TOP };
        qreal posy = (isTop ? TEXTMARGINSTARTY
                            : TEXTMARGINSTARTY + TOPMARGIN + BOARDHEIGHT);
        //        QColor textColor(color == PieceColor::RED ? "#ff0000" : "#1e1e1a");
        for (int col = 0; col < SeatBase::getColNum(); ++col) {
            qreal posx = LEFTWIDTH + TEXTMARGINSTARTX + col * PieceItem::diameter();
            int num = (isTop ? col : SeatBase::symmetryCol(col)) + 1;
            QChar colChar = PieceBase::getNumChar(color, num);
            auto textItem = scene()->addText(QString(colChar), font);
            textItem->setPos(posx, posy);
        }
    }
}

void BoardView::creatPieceItems()
{
    QList<Piece*> allPieces { manual_->getAllPiece() };
    int colorPieceNum = allPieces.size() / 2;
    std::function<QPointF(Piece*)>
        getOriginPos_ = [&](Piece* piece) {
            int colNum = (LEFTWIDTH - PieceItem::diameter()) / PieceItem::halfDiameter();
            int index = allPieces.indexOf(piece) % colorPieceNum;
            Coord coord(index / colNum, index % colNum);
            if (manual_->getHomeSide(piece->color()) == SeatSide::TOP)
                coord = SeatBase::changeCoord(coord, ChangeType::SYMMETRY_V);
            return getScenePos(coord, SCENESTARTX, PieceItem::halfDiameter(),
                SCENESTARTY, PieceItem::diameter());
        };

    pieceParentItem->setData(0, pieceImageDir);
    for (auto& piece : allPieces) {
        PieceItem* item = new PieceItem(getOriginPos_(piece), piece, pieceParentItem);
        item->setFlags(QGraphicsItem::ItemIsMovable
            | QGraphicsItem::ItemIsSelectable
            | QGraphicsItem::ItemIsFocusable);
    }

    allPieceToLeave();
}

QList<PieceItem*> BoardView::getPieceItems() const
{
    QList<PieceItem*> pieceItems;
    for (auto& item : pieceParentItem->childItems())
        pieceItems.append(qgraphicsitem_cast<PieceItem*>(item));

    return pieceItems;
}

QPointF BoardView::getSeatPos(int index) const
{
    return getSeatPos(SeatBase::getCoord(index));
}

QPointF BoardView::getSeatPos(const Coord& coord) const
{
    return getScenePos(coord, BOARDSTARTX, PieceItem::diameter(), SCENESTARTY, PieceItem::diameter());
}

Coord BoardView::getCoord(const QPointF& pos) const
{
    int diameter = PieceItem::diameter(),
        col = int(pos.x() - LEFTWIDTH) / diameter,
        row = int(pos.y()) / diameter;

    return SeatBase::changeCoord({ row, col }, ChangeType::SYMMETRY_V);
}

QPointF BoardView::getScenePos(const Coord& showCoord,
    qreal startX, qreal spacingX, qreal startY, qreal spacingY)
{
    Coord seatCoord = SeatBase::changeCoord(showCoord, ChangeType::SYMMETRY_V);
    return { startX + seatCoord.second * spacingX, startY + seatCoord.first * spacingY };
}
