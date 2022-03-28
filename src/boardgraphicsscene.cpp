#include "boardgraphicsscene.h"
#include "instance.h"
#include "piece.h"
#include "piecegraphicsitem.h"
#include "publicString.h"
#include "seat.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QSettings>

static const qreal hintZValue { 4 };

BoardGraphicsScene::BoardGraphicsScene(int leftWidth, int boardWidth, int boardHeight, Instance* ins)
    : QGraphicsScene()
    , leftWidth_(leftWidth)
    , boardWidth_(boardWidth)
    , boardHeight_(boardHeight)
    , instance(ins)
    , hintParentItem(Q_NULLPTR)
    , pieceParentItem(addRect(sceneRect()))
{
    posStartX = 4;
    posStartY = 3.5;
    posBoardStartX = leftWidth + posStartX;
    pieceDiameter = 57;
    halfDiameter = pieceDiameter / 2;
    setSceneRect(0, 0, leftWidth + boardWidth, boardHeight);

    readSettings();

    creatPieceItems();
}

BoardGraphicsScene::~BoardGraphicsScene()
{
    writeSettings();
}

QPointF BoardGraphicsScene::getLimitPos(const QPointF& pointf) const
{
    bool atBoard = pointf.x() > leftWidth_ - pieceDiameter / 4;
    int startX = atBoard ? leftWidth_ : 0, width = atBoard ? boardWidth_ : leftWidth_,
        maxX = startX + width - pieceDiameter, maxY = boardHeight_ - pieceDiameter;
    return QPointF(pointf.x() < startX ? startX : (pointf.x() > maxX ? maxX : pointf.x()),
        pointf.y() < 0 ? 0 : (pointf.y() > maxY ? maxY : pointf.y()));
}

QPointF BoardGraphicsScene::getSeatPos(int index) const
{
    return getSeatPos(Seats::getSeatCoord(index));
}

QPointF BoardGraphicsScene::getSeatPos(const QPointF& pos) const
{
    return getSeatPos(getSeatCoord(pos));
}

QPointF BoardGraphicsScene::getCenterSeatPos(const QPointF& pos) const
{
    return pos + QPointF(halfDiameter, halfDiameter);
}

SeatCoord BoardGraphicsScene::getSeatCoord(const QPointF& pointf) const
{
    return getSeatCoord((pointf.x() - posBoardStartX) / pieceDiameter,
        (pointf.y() - posStartY) / pieceDiameter);
}

bool BoardGraphicsScene::atBoard(const QPointF& pos) const
{
    return pos.x() > leftWidth_;
}

bool BoardGraphicsScene::canMovePos(const QPointF& fromPos, const QPointF& toPos, QChar ch) const
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

void BoardGraphicsScene::showHintItem(const QPointF& scenePos, QChar ch)
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

void BoardGraphicsScene::clearHintItem()
{
    if (items().contains(hintParentItem))
        removeItem(hintParentItem);
}

void BoardGraphicsScene::allPieceToLeave()
{
    clearFocus();
    shadowItem->leave();
    for (auto item : pieceItemList)
        item->leave();
}

void BoardGraphicsScene::setPieceImageDir()
{
    shadowItem->setImageFile(false);
    for (auto item : pieceItemList)
        item->setImageFile(false);
}

void BoardGraphicsScene::updatePieceItemPos()
{
    // 棋子布局数据
    QString pieceChars = instance->getPieceChars();

    // 标记并移除已在正确位置的棋子和数据
    QVector<bool> pieceUsed(pieceItemList.size(), false);
    int index = 0;
    for (PieceGraphicsItem* item : pieceItemList) {
        int boardIndex = item->boardIndex();
        if (item->atBoard() && pieceChars.at(boardIndex) == item->ch()) {
            pieceUsed[index] = true;
            pieceChars[boardIndex] = Pieces::nullChar;
        }
        ++index;
    }

    // 剩余布局数据找到合适棋子置入该位置
    int boardIndex = 0;
    for (QChar ch : pieceChars) {
        if (ch != Pieces::nullChar) {
            int index = 0;
            for (bool isUsed : pieceUsed) {
                if (!isUsed) {
                    PieceGraphicsItem* item = pieceItemList.at(index);
                    if (item->ch() == ch) {
                        item->setBoardIndex(boardIndex);
                        pieceUsed[index] = true;
                        break;
                    }
                }
                ++index;
            }
        }
        ++boardIndex;
    }

    // 移出全部未作标记者棋子
    index = 0;
    for (bool isUsed : pieceUsed) {
        if (!isUsed)
            pieceItemList.at(index)->leave();
        ++index;
    }

    // 焦点定位至移动棋子，移动走棋标记
    clearFocus();
    shadowItem->leave();
    SeatCoordPair seatCoordPair = instance->getCurSeatCoordPair();
    if (seatCoordPair.first != seatCoordPair.second) {
        QGraphicsItem* item = itemAt(getCenterSeatPos(getSeatPos(seatCoordPair.second)), QTransform());
        if (item) {
            item->setFocus();
            shadowItem->setPos(getSeatPos(seatCoordPair.first));
        }
    }
}

void BoardGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect)

    QRect boardRect(leftWidth_, 0, boardWidth_, boardHeight_);
    painter->drawImage(boardRect, QImage(backImageFile));

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::lightGray, 0, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(QBrush(Qt::lightGray, Qt::SolidPattern));
    painter->drawRect(0, 0, leftWidth_, boardHeight_);
}

void BoardGraphicsScene::writeSettings() const
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::SCENEOPTION]);

    settings.setValue(stringLiterals[StringIndex::LEAVEPIECEORDER], leaveIsTidy);
    settings.setValue(stringLiterals[StringIndex::MOVEANIMATED], moveAnimated);
    settings.setValue(stringLiterals[StringIndex::BACKIMAGEFILE], backImageFile);
    settings.setValue(stringLiterals[StringIndex::PIECEIMAGEDIR], pieceParentItem->data(ItemDataIndex::IMAGEFILETEMP));

    settings.endGroup();
}

void BoardGraphicsScene::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::SCENEOPTION]);

    leaveIsTidy = settings.value(stringLiterals[StringIndex::LEAVEPIECEORDER], true).toBool();
    moveAnimated = settings.value(stringLiterals[StringIndex::MOVEANIMATED], false).toBool();
    backImageFile = settings.value(stringLiterals[StringIndex::BACKIMAGEFILE], "./res/IMAGES_L/WOOD.JPG").toString();
    pieceParentItem->setData(ItemDataIndex::IMAGEFILETEMP,
        settings.value(stringLiterals[StringIndex::PIECEIMAGEDIR], "./res/IMAGES_L/WOOD").toString());

    settings.endGroup();
}

void BoardGraphicsScene::creatPieceItems()
{
    std::function<QPointF(PieceColor, int)>
        getLeavePos_ = [&](PieceColor color, int index) {
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
                PieceGraphicsItem* item = new PieceGraphicsItem(this, pieceParentItem);
                item->setFlags(QGraphicsItem::ItemIsMovable
                    | QGraphicsItem::ItemIsSelectable
                    | QGraphicsItem::ItemIsFocusable);
                item->setCh(ch);
                item->setLeavePos(getLeavePos_(color, index));
                pieceItemList.append(item);

                index++;
            }
            kind++;
        }
    }
    shadowItem = new PieceGraphicsItem(this, pieceParentItem);
    shadowItem->setCh(Pieces::nullChar);
    shadowItem->setLeavePos({ -pieceDiameter, -pieceDiameter });

    allPieceToLeave();
    setPieceImageDir();
}

QPointF BoardGraphicsScene::getSeatPos(const SeatCoord& seatCoord) const
{
    return getScenePos(seatCoord, posBoardStartX, pieceDiameter, posStartY, pieceDiameter);
}

QList<SeatCoord> BoardGraphicsScene::getPutSeatCoordList(QChar ch) const
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

QList<SeatCoord> BoardGraphicsScene::getMoveSeatCoordList(const SeatCoord& seatCoord) const
{
    return instance->canMove(seatCoord);
}

void BoardGraphicsScene::showHintItem(QList<SeatCoord> seatCoordList)
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

SeatCoord BoardGraphicsScene::getSeatCoord(int colX, int rowY)
{
    return Seats::changeSeatCoord({ rowY, colX }, ChangeType::SYMMETRY_V);
}

QPointF BoardGraphicsScene::getScenePos(const SeatCoord& seatCoord,
    qreal startX, qreal spacingX, qreal startY, qreal spacingY)
{
    SeatCoord showSeat = Seats::changeSeatCoord(seatCoord, ChangeType::SYMMETRY_V);
    return { startX + showSeat.second * spacingX, startY + showSeat.first * spacingY };
}
