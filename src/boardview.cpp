
#include "boardview.h"
#include "boardpieces.h"
#include "boardscene.h"
#include "common.h"
#include "manual.h"
#include "manualmove.h"
#include "manualsubwindow.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "pieceitem.h"
#include "seat.h"
#include "seatbase.h"
#include "tools.h"

#include <QDebug>
#include <QMouseEvent>
#include <QSettings>

#define TOPMARGIN 28
#define BOTTOMMARGIN 28
#define LEFTWIDTH 200
#define BOARDWIDTH 521
#define BOARDHEIGHT 577
#define STARTXY 3
#define BOARDSTARTX (LEFTWIDTH + STARTXY)
#define BOARDSTARTY (TOPMARGIN + STARTXY)
#define TEXTMARGINSTARTX 20
#define TEXTMARGINSTARTY 0

#define NOTBOARDINDEX (-1)
#define OUTSIZE (-1000)

static int topMargin() { return TOPMARGIN * Tools::getReviseScale(); }
static int bottomMargin() { return BOTTOMMARGIN * Tools::getReviseScale(); }
static int leftWidth() { return LEFTWIDTH * Tools::getReviseScale(); }
static int boardWidth() { return BOARDWIDTH * Tools::getReviseScale(); }
static int boardHeight() { return BOARDHEIGHT * Tools::getReviseScale(); }
static int startXY() { return STARTXY * Tools::getReviseScale(); }
static int boardStartX() { return BOARDSTARTX * Tools::getReviseScale(); }
static int boardStartY() { return BOARDSTARTY * Tools::getReviseScale(); }
static int textMarginStartX() { return TEXTMARGINSTARTX * Tools::getReviseScale(); }
static int textMarginStartY() { return TEXTMARGINSTARTY * Tools::getReviseScale(); }

static QRect getBoardRect()
{
    return QRect(leftWidth(), topMargin(), boardWidth(), boardHeight());
}

static QRectF getSceneRect()
{
    return QRectF(0, 0, leftWidth() + boardWidth(), topMargin() + boardHeight() + bottomMargin());
}

BoardView::BoardView(QWidget* parent)
    : QGraphicsView(parent)
{
    //    setScene(new BoardScene(QRectF(0, 0, LEFTWIDTH + BOARDWIDTH, BOARDHEIGHT), this));
    setScene(new BoardScene(getSceneRect(), this));
    scene()->setBackgroundBrush(QBrush(Qt::lightGray, Qt::SolidPattern));

    hintParentItem = scene()->addRect(QRect(), Qt::NoPen);
    pieceParentItem = scene()->addRect(QRect(), Qt::NoPen);

    readSettings();
    shadowItem = new QGraphicsPixmapItem(QPixmap(QString("%1/OOS.GIF").arg(pieceImageDir)));
    shadowItem->setScale(Tools::getReviseScale());
    scene()->addItem(shadowItem);
}

BoardView::~BoardView()
{
    writeSettings();
}

void BoardView::setManualSubWindow(ManualSubWindow* manualSubWindow)
{
    manualSubWindow_ = manualSubWindow;
    creatMarginItems();
    creatPieceItems();
}

QRect BoardView::boardRect() const
{
    return getBoardRect();
}

bool BoardView::atBoard(const QPointF& pos) const
{
    return pos.x() > leftWidth();
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

QPointF BoardView::getLocatePos(const QPointF& seatPos) const
{
    bool atBoard = seatPos.x() > leftWidth() - PieceItem::diameter() / 4;
    int startX = atBoard ? leftWidth() : 0,
        width = atBoard ? boardWidth() : leftWidth(),
        maxX = startX + width - PieceItem::diameter(),
        maxY = topMargin() + boardHeight() - PieceItem::diameter();
    return QPoint(seatPos.x() < startX ? startX : (seatPos.x() > maxX ? maxX : seatPos.x()),
        seatPos.y() < topMargin() ? topMargin() : (seatPos.y() > maxY ? maxY : seatPos.y()));
}

QPointF BoardView::getMovedPos(Piece* piece, const QPointF& fromSeatPos,
    const QPointF& toPos, const QPointF& mousePos) const
{
    bool allowMove { true };
    Coord fromCoord = getCoord(fromSeatPos), toCoord = getCoord(toPos);
    bool fromAtBoard { atBoard(fromSeatPos) }, toAtBoard { atBoard(toPos) };
    if (fromAtBoard != toAtBoard)
        allowMove = manualSubWindow_->setState(SubWinState::LAYOUT);

    // 目标位置在棋盘上，判断是否在允许范围
    if (allowMove && toAtBoard) {
        allowMove = manualSubWindow_->getAllowCoords(piece, fromCoord, fromAtBoard).contains(toCoord);
        // 起点和目标都在棋盘上，且在允许范围
        if (fromAtBoard && allowMove
            && (allowMove = manualSubWindow_->setState(SubWinState::PLAY)))
            allowMove = manualSubWindow_->appendMove({ fromCoord, toCoord });
    }

    return (allowMove ? (toAtBoard ? getSeatPos(toCoord) : getLocatePos(toPos - mousePos)) : fromSeatPos);
}

void BoardView::showHintItem(Piece* piece, const QPointF& fromSeatPos)
{
    QList<Coord> coords { manualSubWindow_->getAllowCoords(piece, getCoord(fromSeatPos), atBoard(fromSeatPos)) };
    if (coords.isEmpty())
        return;

    bool isLayout { manualSubWindow_->isState(SubWinState::LAYOUT) };
    qreal radius = PieceItem::diameter() * 2 / 3;
    qreal startX { boardStartX() + PieceItem::diameter() / 6 };
    qreal startY { boardStartY() + PieceItem::diameter() / 6 };
    qreal spacing { PieceItem::diameter() };
    QRectF rect(0, 0, radius, radius);
    QPen pen(isLayout ? Qt::darkGreen : Qt::blue, isLayout ? 2 : 3, Qt::DashLine, Qt::RoundCap);
    //    QBrush(Qt::lightGray, Qt::Dense6Pattern);
    for (const Coord& coord : coords) {
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(rect, hintParentItem);
        item->setPen(pen);
        item->setPos(getScenePos(coord, startX, spacing, startY, spacing));
        item->setZValue(HINTZVALUE);
    }
}

void BoardView::clearHintItem()
{
    for (auto& item : hintParentItem->childItems()) {
        scene()->removeItem(item);
        delete item;
    }
}

void BoardView::updateShowPieceItem()
{
    Seat* curSeat {};
    if (!manualSubWindow_->manual()->manualMove()->move()->isRoot()) {
        SeatPair seatPair = manualSubWindow_->manual()->manualMove()->curSeatPair();
        shadowItem->setPos(getSeatPos(seatPair.first->coord()));
        curSeat = seatPair.second;
    } else
        shadowItem->setPos(QPointF(OUTSIZE, OUTSIZE));

    scene()->clearSelection();
    QList<Seat*> liveSeats { manualSubWindow_->manual()->getLiveSeats() };
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
                    item->moveToPos(getSeatPos(coord));
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
        bool isTop { manualSubWindow_->manual()->getHomeSide(color) == SeatSide::TOP };
        qreal posy = (isTop ? textMarginStartY()
                            : textMarginStartY() + topMargin() + boardHeight());
        //        QColor textColor(color == PieceColor::RED ? "#ff0000" : "#1e1e1a");
        for (int col = 0; col < SeatBase::getColNum(); ++col) {
            qreal posx = leftWidth() + textMarginStartX() + col * PieceItem::diameter();
            int num = (isTop ? col : SeatBase::symmetryCol(col)) + 1;
            QChar colChar = PieceBase::getNumChar(color, num);
            auto textItem = scene()->addText(QString(colChar), font);
            textItem->setPos(posx, posy);
        }
    }
}

void BoardView::creatPieceItems()
{
    QList<Piece*> allPieces { manualSubWindow_->manual()->getAllPieces() };
    int colorPieceNum = allPieces.size() / 2;
    std::function<QPointF(Piece*)>
        getOriginPos_ = [&](Piece* piece) {
            int colNum = (leftWidth() - PieceItem::diameter()) / PieceItem::halfDiameter();
            int index = allPieces.indexOf(piece) % colorPieceNum;
            Coord coord(index / colNum, index % colNum);
            if (manualSubWindow_->manual()->getHomeSide(piece->color()) == SeatSide::TOP)
                coord = SeatBase::changeCoord(coord, ChangeType::SYMMETRY_V);
            return getScenePos(coord, startXY(), PieceItem::halfDiameter(),
                boardStartY(), PieceItem::diameter());
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
    return getScenePos(coord, boardStartX(), PieceItem::diameter(), boardStartY(), PieceItem::diameter());
}

Coord BoardView::getCoord(const QPointF& pos) const
{
    int col = int(pos.x() + 3 - leftWidth()) / PieceItem::diameter(),
        row = int(pos.y() + 3 - topMargin()) / PieceItem::diameter();

    return SeatBase::changeCoord({ row, col }, ChangeType::SYMMETRY_V);
}

QPointF BoardView::getScenePos(const Coord& showCoord,
    qreal startX, qreal spacingX, qreal startY, qreal spacingY)
{
    Coord seatCoord = SeatBase::changeCoord(showCoord, ChangeType::SYMMETRY_V);
    return { startX + seatCoord.second * spacingX, startY + seatCoord.first * spacingY };
}
