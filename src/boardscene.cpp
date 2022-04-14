#include "boardscene.h"
#include "instance.h"
#include "piece.h"
#include "pieceitem.h"
#include "publicString.h"
#include "seat.h"
#include <QDebug>
#include <QPainter>
#include <QRandomGenerator>
#include <QSettings>

static const qreal hintZValue { 4 };

BoardScene::BoardScene(Instance* ins, QObject* parent)
    : QGraphicsScene(parent)
    , instance(ins)
    , hintParentItem(Q_NULLPTR)
    , pieceParentItem(addRect(sceneRect()))
{
    setSceneRect(0, 0, leftWidth_ + boardWidth_, boardHeight_);

    readSettings();
    creatPieceItems();
}

BoardScene::~BoardScene()
{
    writeSettings();
}

QPointF BoardScene::getLimitPos(const QPointF& pointf) const
{
    bool atBoard = pointf.x() > leftWidth_ - pieceDiameter / 4;
    int startX = atBoard ? leftWidth_ : 0, width = atBoard ? boardWidth_ : leftWidth_,
        maxX = startX + width - pieceDiameter, maxY = boardHeight_ - pieceDiameter;
    return QPointF(pointf.x() < startX ? startX : (pointf.x() > maxX ? maxX : pointf.x()),
        pointf.y() < 0 ? 0 : (pointf.y() > maxY ? maxY : pointf.y()));
}

QPointF BoardScene::getSeatPos(int index) const
{
    return getSeatPos(Seats::getSeatCoord(index));
}

QPointF BoardScene::getSeatPos(const QPointF& pos) const
{
    return getSeatPos(getSeatCoord(pos));
}

int BoardScene::getBoardIndex(const QPointF& pos) const
{
    if (pos.x() < posBoardStartX)
        return NOTBOARDINDEX;

    return Seats::getBoardIndex(getSeatCoord(pos));
}

QPointF BoardScene::getCenterSeatPos(const QPointF& pos) const
{
    return pos + QPointF(halfDiameter, halfDiameter);
}

SeatCoord BoardScene::getSeatCoord(const QPointF& pos) const
{
    return getSeatCoord((pos.x() - posBoardStartX) / pieceDiameter,
        (pos.y() - posStartY) / pieceDiameter);
}

bool BoardScene::atBoard(const QPointF& pos) const
{
    return pos.x() > leftWidth_;
}

bool BoardScene::canMovePos(const QPointF& fromPos, const QPointF& toPos, QChar ch) const
{
    SeatCoord toSeatCoord = getSeatCoord(toPos);
    bool fromAtBoard { atBoard(fromPos) }, toAtBoard { atBoard(toPos) };
    switch (instance->status()) {
    case InsStatus::LAYOUT:
        return toAtBoard ? getPutSeatCoordList(ch).contains(toSeatCoord) : true;
    case InsStatus::PLAY:
        if (fromAtBoard && toAtBoard
            && getMoveSeatCoordList(getSeatCoord(fromPos)).contains(toSeatCoord))
            return true;

        return !fromAtBoard && !toAtBoard;
    case InsStatus::MOVEDEMO:
        break;
    default:
        break;
    }

    return false;
}

void BoardScene::showHintItem(const QPointF& scenePos, QChar ch)
{
    switch (instance->status()) {
    case InsStatus::LAYOUT:
        showHintItem(getPutSeatCoordList(ch));
        break;
    case InsStatus::PLAY:
        if (atBoard(scenePos))
            showHintItem(getMoveSeatCoordList(getSeatCoord(scenePos)));
        break;
    case InsStatus::MOVEDEMO:
        break;
    default:
        break;
    }
}

void BoardScene::clearHintItem()
{
    if (items().contains(hintParentItem))
        removeItem(hintParentItem);
}

void BoardScene::allPieceToLeave()
{
    clearFocus();
    shadowItem->leave();
    for (auto item : pieceItemList)
        item->leave();
}

void BoardScene::updatePieceItemShow()
{
    // 当前着法移动的棋子
    int curMoveBoardIndex = NOTBOARDINDEX;
    if (!instance->isStartMove()) {
        SeatCoordPair seatCoordPair = instance->getCurSeatCoordPair();
        curMoveBoardIndex = Seats::getBoardIndex(seatCoordPair.second);
        shadowItem->setPos(getSeatPos(seatCoordPair.first));
    } else
        shadowItem->leave();

    clearFocus();
    QString pieceChars = instance->getPieceChars();
    QList<PieceItem*> copyPieceItemList { pieceItemList };
    QList<QPair<int, QChar>> indexChList;
    PieceItem* pieceItem;
    int boardIndex = 0;
    for (QChar& ch : pieceChars) {
        if (ch != Pieces::nullChar) {
            QGraphicsItem* item = itemAt(getSeatPos(boardIndex), QTransform());
            if (item && (pieceItem = dynamic_cast<PieceItem*>(item))) {
                copyPieceItemList.removeOne(pieceItem);
                if (pieceItem->ch() == ch) {
                    ch = Pieces::nullChar;
                } else {
                    copyPieceItemList.prepend(pieceItem);
                }
            }
        }
        boardIndex++;
    }

    boardIndex = 0;
    for (QChar& ch : pieceChars) {
        if (ch != Pieces::nullChar) {
            bool find { false };
            QMutableListIterator<PieceItem*> iterator(copyPieceItemList);
            while (iterator.hasNext()) {
                auto& item = iterator.next();
                if (item->ch() == ch) {
                    item->setScenePos(getSeatPos(boardIndex));
                    // 前进时，当前着法棋子获得焦点
                    if (boardIndex == curMoveBoardIndex)
                        item->setFocus();

                    iterator.remove();
                    find = true;
                    break;
                }
            }
            Q_ASSERT(find);
        }
        boardIndex++;
    }

    for (auto& item : copyPieceItemList)
        item->leave();
}

void BoardScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect)

    //    painter->drawImage(boardSceneRect(), QImage(backImageFile));
    painter->drawImage(boardSceneRect(), QImage(":/res/IMAGES_L/WOOD.JPG"));

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::lightGray, 0, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    painter->drawRect(leaveSceneRect());
}

void BoardScene::writeSettings() const
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::SCENEOPTION]);

    settings.setValue(stringLiterals[StringIndex::LEAVEPIECEORDER], leaveIsTidy);
    settings.setValue(stringLiterals[StringIndex::MOVEANIMATED], moveAnimated);
    settings.setValue(stringLiterals[StringIndex::BACKIMAGEFILE], backImageFile);
    settings.setValue(stringLiterals[StringIndex::PIECEIMAGEDIR], pieceParentItem->data(DataIndex::IMAGEDIR));

    settings.endGroup();
}

void BoardScene::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::SCENEOPTION]);

    leaveIsTidy = settings.value(stringLiterals[StringIndex::LEAVEPIECEORDER], true).toBool();
    moveAnimated = settings.value(stringLiterals[StringIndex::MOVEANIMATED], false).toBool();
    backImageFile = settings.value(stringLiterals[StringIndex::BACKIMAGEFILE], ":/res/IMAGES_L/WOOD.JPG").toString();
    pieceParentItem->setData(DataIndex::IMAGEDIR,
        settings.value(stringLiterals[StringIndex::PIECEIMAGEDIR], ":/res/IMAGES_L/WOOD").toString());

    settings.endGroup();
}

void BoardScene::creatPieceItems()
{
    std::function<QPointF(PieceColor, int)>
        getOriginPos_ = [&](PieceColor color, int index) {
            int colNum = (leftWidth_ - pieceDiameter) / halfDiameter;
            SeatCoord seatCoord(index / colNum, index % colNum);
            if (instance->getHomeSide(color) == SeatSide::TOP)
                seatCoord = getSeatCoord(seatCoord.second, seatCoord.first);
            return getScenePos(seatCoord, posStartX, halfDiameter, posStartY, pieceDiameter);
        };

    QList<int> kindNum = { 1, 2, 2, 2, 2, 2, 5 };
    for (PieceColor color : { PieceColor::RED, PieceColor::BLACK }) {
        int kind = 0, index = 0;
        for (QChar ch : Pieces::chars[int(color)]) {
            for (int i = 0; i < kindNum.at(kind); ++i) {
                PieceItem* item = new PieceItem(ch, getOriginPos_(color, index),
                    this, pieceParentItem);
                item->setFlags(QGraphicsItem::ItemIsMovable
                    | QGraphicsItem::ItemIsSelectable
                    | QGraphicsItem::ItemIsFocusable);
                pieceItemList.append(item);

                index++;
            }
            kind++;
        }
    }
    shadowItem = new PieceItem(Pieces::nullChar,
        { -pieceDiameter, -pieceDiameter }, this, pieceParentItem);

    allPieceToLeave();
}

QPointF BoardScene::getSeatPos(const SeatCoord& seatCoord) const
{
    return getScenePos(seatCoord, posBoardStartX, pieceDiameter, posStartY, pieceDiameter);
}

QList<SeatCoord> BoardScene::getPutSeatCoordList(QChar ch) const
{
    SeatSide homeSide = instance->getHomeSide(Pieces::getColor(ch));
    switch (Pieces::getKind(ch)) {
    case PieceKind::KING:
        return Seats::kingPutTo(homeSide);
    case PieceKind::ADVISOR:
        return Seats::advisorPutTo(homeSide);
    case PieceKind::BISHOP:
        return Seats::bishopPutTo(homeSide);
    case PieceKind::PAWN:
        return Seats::pawnPutTo(homeSide);
    default:
        break;
    }

    return Seats::allSeatCoord();
}

QList<SeatCoord> BoardScene::getMoveSeatCoordList(const SeatCoord& seatCoord) const
{
    return instance->canMove(seatCoord);
}

void BoardScene::showHintItem(QList<SeatCoord> seatCoordList)
{
    qreal radius = pieceDiameter * 2 / 3;
    QRectF rect(0, 0, radius, radius);
    QPen pen(instance->status() == InsStatus::PLAY ? Qt::blue : Qt::darkGreen,
        instance->status() == InsStatus::PLAY ? 3 : 2, Qt::DashLine, Qt::RoundCap);
    //    QBrush(Qt::lightGray, Qt::Dense6Pattern);
    hintParentItem = addRect(sceneRect());
    for (SeatCoord& seatCoord : seatCoordList) {
        QGraphicsEllipseItem* item = new QGraphicsEllipseItem(rect, hintParentItem);
        item->setPen(pen);
        item->setPos(getScenePos(seatCoord,
            posBoardStartX + pieceDiameter / 6, pieceDiameter,
            posStartY + pieceDiameter / 6, pieceDiameter));
        item->setZValue(hintZValue);
    }
}

SeatCoord BoardScene::getSeatCoord(int colX, int rowY)
{
    return Seats::changeSeatCoord({ rowY, colX }, ChangeType::SYMMETRY_V);
}

QPointF BoardScene::getScenePos(const SeatCoord& seatCoord,
    qreal startX, qreal spacingX, qreal startY, qreal spacingY)
{
    SeatCoord showSeat = Seats::changeSeatCoord(seatCoord, ChangeType::SYMMETRY_V);
    return { startX + showSeat.second * spacingX, startY + showSeat.first * spacingY };
}
