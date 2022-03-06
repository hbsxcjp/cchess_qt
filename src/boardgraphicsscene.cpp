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
    , insStatus(InsStatus::INITLAYOUT)
    , instance(ins)
    , pieceItemList(QList<PieceGraphicsItem*>())
    , hintItemList(QList<QGraphicsItem*>())
{
    pointStartX = 4;
    pointStartY = 3.5;
    pointBoardStartX = leftWidth + pointStartX;
    pieceDiameter = 57;
    halfDiameter = pieceDiameter / 2;
    setSceneRect(0, 0, leftWidth + boardWidth, boardHeight);

    readSettings();
}

BoardGraphicsScene::~BoardGraphicsScene()
{
    writeSettings();
}

bool BoardGraphicsScene::atBoard(const QPointF& pointf) const
{
    return pointf.x() > leftWidth_ && pointf.x() < sceneRect().width();
}

bool BoardGraphicsScene::atLeave(const QPointF& pointf) const
{
    return pointf.x() > 0 && pointf.x() < leftWidth_;
}

QPointF BoardGraphicsScene::getBoardPointF(const QPointF& itemPointf) const
{
    return getLimitOriginPointF(getLimitCenterPointF(itemPointf, leftWidth_, boardWidth_, boardHeight_));
}

QPointF BoardGraphicsScene::getLeavePointF(const QPointF& itemPointf) const
{
    return getLimitOriginPointF(getLimitCenterPointF(itemPointf, 0, leftWidth_, boardHeight_));
}

QPointF BoardGraphicsScene::getHintPointF(const SeatCoord& seatCoord) const
{
    return getPointF(seatCoord, pointBoardStartX + pieceDiameter / 6, pieceDiameter,
        pointStartY + pieceDiameter / 6, pieceDiameter);
}

void BoardGraphicsScene::loadPieceItems()
{
    creatPieceItems();
    setPieceItemImageTheme(pieceImageDir);
    setPieceItemPoses();
}

void BoardGraphicsScene::setPieceItemImageTheme(const QString& pieceImageDir)
{
    QString fileNameTemp { QString(":/res/IMAGES_L/%1/%2%3.GIF").arg(pieceImageDir) };
    for (auto item : pieceItemList) {
        QChar ch = item->ch(),
              colorChar = ch.isUpper() ? 'R' : 'B';
        item->setPixmap(QPixmap(fileNameTemp.arg(colorChar).arg(ch.toUpper())));
    }
}

void BoardGraphicsScene::setBoardImageTheme(const QString& imageFile)
{
    backImageFile = imageFile;
    // 重新建立场景？
}

QList<SeatCoord> BoardGraphicsScene::getPutSeatCoordList(PieceGraphicsItem* item) const
{
    QChar ch = item->ch();
    SeatSide showHomeSide = Seats::showHomeSide(instance->getHomeSide(Pieces::getColor(ch)));
    switch (Pieces::getKind(ch)) {
    case PieceKind::KING:
        return Seats::kingPutTo(showHomeSide);
    case PieceKind::ADVISOR:
        return Seats::advisorPutTo(showHomeSide);
    case PieceKind::BISHOP:
        return Seats::bishopPutTo(showHomeSide);
    case PieceKind::PAWN:
        return Seats::pawnPutTo(showHomeSide);
    default:
        break;
    }

    return Seats::allSeatCoord();
}

void BoardGraphicsScene::setHintItemList(QList<SeatCoord> seatCoordList)
{
    qreal radius = pieceDiameter * 2 / 3;
    QRectF rect(0, 0, radius, radius);
    QPen pen(Qt::blue, 3, Qt::DashLine, Qt::RoundCap);
    //    QBrush(Qt::lightGray, Qt::Dense6Pattern);
    for (SeatCoord& seatCoord : seatCoordList) {
        QGraphicsItem* item = addEllipse(rect, pen);
        item->setPos(getHintPointF(seatCoord));
        item->setZValue(hintZValue);

        hintItemList.append(item);
    }
}

void BoardGraphicsScene::clearHintItemList()
{
    for (QGraphicsItem* item : hintItemList)
        removeItem(item);
}

void BoardGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect)

    QRect boardRect(leftWidth_, 0, boardWidth_, boardHeight_);
    QString imageFile { QString(":/res/IMAGES_L/%1").arg(backImageFile) };
    painter->drawImage(boardRect, QImage(imageFile));

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
    settings.setValue(stringLiterals[StringIndex::MOVESOUND], moveSound);
    settings.setValue(stringLiterals[StringIndex::BACKIMAGEFILE], backImageFile);
    settings.setValue(stringLiterals[StringIndex::PIECEIMAGEDIR], pieceImageDir);

    settings.endGroup();
}

void BoardGraphicsScene::readSettings()
{
    QSettings settings;
    settings.beginGroup(stringLiterals[StringIndex::SCENEOPTION]);

    leaveIsTidy = settings.value(stringLiterals[StringIndex::LEAVEPIECEORDER], true).toBool();
    moveAnimated = settings.value(stringLiterals[StringIndex::MOVEANIMATED], false).toBool();
    moveSound = settings.value(stringLiterals[StringIndex::MOVESOUND], true).toBool();
    backImageFile = settings.value(stringLiterals[StringIndex::BACKIMAGEFILE], "WOOD.JPG").toString();
    pieceImageDir = settings.value(stringLiterals[StringIndex::PIECEIMAGEDIR], "WOOD").toString();

    settings.endGroup();
}

PieceGraphicsItem* BoardGraphicsScene::getLeavePieceItem(QChar ch) const
{
    for (auto item : pieceItemList) {
        if (!atBoard(item->scenePos())
            && item->ch() == ch)
            return item;
    }

    return Q_NULLPTR;
}

QPointF BoardGraphicsScene::getLimitCenterPointF(const QPointF& itemPointf,
    int leftWidth, int width, int height) const
{
    qreal x = itemPointf.x(),
          y = itemPointf.y();
    int minX = leftWidth + halfDiameter,
        maxX = leftWidth + width - halfDiameter,
        minY = halfDiameter,
        maxY = height - halfDiameter;
    return { x < minX ? minX : (x > maxX ? maxX : x), y < minY ? minY : (y > maxY ? maxY : y) };
}

QPointF BoardGraphicsScene::getLimitOriginPointF(const QPointF& itemPointf) const
{
    return { itemPointf.x() - halfDiameter, itemPointf.y() - halfDiameter };
}

QPointF BoardGraphicsScene::getSeatPointF(const SeatCoord& seatCoord) const
{
    return getPointF(seatCoord, pointBoardStartX, pieceDiameter, pointStartY, pieceDiameter);
}

QPointF BoardGraphicsScene::getLeavePointF(const SeatCoord& seatCoord) const
{
    return getPointF(seatCoord, pointStartX, halfDiameter, pointStartY, pieceDiameter);
}

SeatCoord BoardGraphicsScene::getSeatCoord(const QPointF& pointf) const
{
    return getCoord(pointf, pointBoardStartX, pieceDiameter, pointStartY, pieceDiameter);
}

SeatCoord BoardGraphicsScene::getLeaveCoord(const QPointF& pointf) const
{
    return getCoord(pointf, pointStartX, halfDiameter, pointStartY, pieceDiameter);
}

SeatCoord BoardGraphicsScene::showSeatCoord(const SeatCoord& seatCoord)
{
    return Seats::changeSeatCoord(seatCoord, ChangeType::SYMMETRY_V);
}

QPointF BoardGraphicsScene::getPointF(const SeatCoord& seatCoord,
    qreal startX, qreal spacingX, qreal startY, qreal spacingY)
{
    return { startX + seatCoord.second * spacingX, startY + seatCoord.first * spacingY };
}

SeatCoord BoardGraphicsScene::getCoord(const QPointF& pointf,
    qreal startX, qreal spacingX, qreal startY, qreal spacingY)
{
    return SeatCoord((pointf.y() - startY) / spacingY, (pointf.x() - startX) / spacingX);
}

void BoardGraphicsScene::creatPieceItems()
{
    for (QChar ch : Pieces::allChars) {
        PieceGraphicsItem* item = new PieceGraphicsItem(this);
        item->setFlags(QGraphicsItem::ItemIsMovable
            | QGraphicsItem::ItemIsSelectable
            | QGraphicsItem::ItemIsFocusable);
        //        SeatSide homeSide = instance->getHomeSide(color);
        item->setCh(ch);
        item->setPos(getNextLeavePointf(item->color()));

        addItem(item);
        pieceItemList.append(item);
    }
}

void BoardGraphicsScene::setPieceItemPoses()
{
    int index = 0;
    PieceGraphicsItem* pieceItem;
    for (QChar ch : instance->getPieceChars()) {
        if (ch != Pieces::nullChar && (pieceItem = getLeavePieceItem(ch)))
            pieceItem->setPos(getSeatPointF(showSeatCoord(SeatCoord(index / SEATCOL, index % SEATCOL))));
        index++;
    }
}

int BoardGraphicsScene::getLeavePieceNum(PieceColor color)
{
    int num = 0;
    for (auto item : pieceItemList) {
        if (item->color() == color && atLeave(item->scenePos()))
            num++;
    }

    return num;
}

QPointF BoardGraphicsScene::getNextLeavePointf(PieceColor color)
{
    SeatCoord seatCoord;
    int rowNum = SEATROW / 2,
        colNum = (leftWidth_ - pieceDiameter) / halfDiameter;
    if (leaveIsTidy) {
        int num = getLeavePieceNum(color);
        seatCoord = { num / colNum, num % colNum };
        // 如在底部，行数倒置
        if (instance->getHomeSide(color) == SeatSide::BOTTOM)
            seatCoord = showSeatCoord(seatCoord);
    } else {
        int rowRandNum = QRandomGenerator::global()->generate(),
            colRandNum = QRandomGenerator::global()->generate();
        seatCoord = SeatCoord(rowRandNum % rowNum, colRandNum % colNum);
    }

    return getLeavePointF(seatCoord);
}
