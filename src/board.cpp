#include "board.h"
#include "aspect.h"
#include "boardpieces.h"
#include "boardseats.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"
#include "tools.h"

Board::Board()
    : boardPieces_(new BoardPieces)
    , boardSeats_(new BoardSeats)
    , bottomColor_(PieceColor::RED)
{
    init();
}

Board::~Board()
{
    delete boardPieces_;
    delete boardSeats_;
}

void Board::init()
{
    for (auto& coordPiece : SeatBase::getInitCoordColorKinds()) {
        PieceColor color { coordPiece.second.first };
        PieceKind kind { coordPiece.second.second };
        boardSeats_->getSeat(coordPiece.first)->setPiece(boardPieces_->getNonLivePiece(color, kind));
    }

    bottomColor_ = PieceColor::RED;
}

QList<Piece*> Board::getAllPiece() const
{
    return boardPieces_->getAllPiece();
}

QList<Seat*> Board::getLiveSeats() const
{
    return boardPieces_->getLiveSeats();
}

Piece* Board::getPiece(Coord coord) const
{
    return boardSeats_->getSeat(coord)->piece();
}

QList<Coord> Board::getLiveSeatCoordList(PieceColor color) const
{
    return getCoordList(boardPieces_->getLiveSeats(color));
}

QList<QList<Coord>> Board::canMove(const Coord& fromCoord) const
{
    return canMove(boardSeats_->getSeat(fromCoord));
}

QMap<Seat*, QList<Coord>> Board::allCanMove(PieceColor color) const
{
    QMap<Seat*, QList<Coord>> seatCoords;
    for (auto& fromSeat : boardPieces_->getLiveSeats(color))
        seatCoords[fromSeat] = canMove(fromSeat).value(0);

    return seatCoords;
}

bool Board::isCanMove(const SeatPair& seatPair) const
{
    return canMove(seatPair.first).value(0).contains(seatPair.second->coord());
}

bool Board::isFace() const
{
    Seat* redSeat = boardPieces_->getKingSeat(PieceColor::RED);
    Seat* blackSeat = boardPieces_->getKingSeat(PieceColor::BLACK);
    Q_ASSERT(redSeat && blackSeat);

    return boardSeats_->isFace(redSeat, blackSeat);
}

bool Board::isKilled(PieceColor color) const
{
    Seat* kingSeat = boardPieces_->getKingSeat(color);
    Q_ASSERT(kingSeat);

    Coord kingCoord = kingSeat->coord();
    PieceColor otherColor = PieceBase::getOtherColor(color);
    SeatSide otherHomeSide = getHomeSide(otherColor);
    for (auto& fromSeat : boardPieces_->getLiveSeats(otherColor)) {
        auto allCoords = fromSeat->canMove(boardSeats_, otherHomeSide);
        if (allCoords.value(0).contains(kingCoord))
            return true;
    }

    return false;
}

bool Board::isFailed(PieceColor color) const
{
    for (auto& fromSeat : boardPieces_->getLiveSeats(color))
        if (!canMove(fromSeat).value(0).isEmpty())
            return false;

    return true;
}

QString Board::getPieceChars() const
{
    return boardSeats_->getPieceChars();
}

QString Board::getFEN() const
{
    return boardSeats_->getFEN();
}

bool Board::setFEN(const QString& fen)
{
    return boardSeats_->setFEN(boardPieces_, fen) && setBottomColor();
}

void Board::changeSeatPair(SeatPair& seatPair, ChangeType ct) const
{
    seatPair = { boardSeats_->changeSeat(seatPair.first, ct),
        boardSeats_->changeSeat(seatPair.second, ct) };
}

bool Board::changeLayout(ChangeType ct)
{
    boardSeats_->changeLayout(boardPieces_, ct);
    return setBottomColor();
}

QString Board::getZhStr(const SeatPair& seatPair) const
{
    Seat* fseat { seatPair.first };
    Seat* tseat { seatPair.second };
    if (!fseat->hasPiece())
        return {};

    QString zhStr;
    Piece* fromPiece { fseat->piece() };
    PieceColor color { fromPiece->color() };
    QChar name { fromPiece->name() };
    int fromRow { fseat->row() }, fromCol { fseat->col() },
        toRow { tseat->row() }, toCol { tseat->col() };
    bool isSameRow { fromRow == toRow }, isBottom { color == bottomColor_ };
    QList<Seat*> seats = boardPieces_->getLiveSeats(color, name, fromCol);

    if (seats.size() > 1 && PieceBase::isStrongePiece(name)) {
        bool isPawn = PieceBase::isPawnPiece(name);
        if (isPawn)
            seats = boardPieces_->getLiveSeats_SortPawn(color, isBottom);
        else
            // 按先行后列的顺序，从小到大排序
            std::sort(seats.begin(), seats.end(), SeatBase::less);

        // 如是兵，已根据是否底边排好序
        zhStr.append(PieceBase::getIndexChar(seats.size(),
                         isPawn ? false : isBottom, seats.indexOf(fseat)))
            .append(name);
    } else { //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        zhStr.append(name)
            .append(PieceBase::getColChar(color, isBottom, fromCol));
    }
    zhStr.append(PieceBase::getMovChar(isSameRow, isBottom, toRow > fromRow))
        .append(PieceBase::isLinePiece(name) && !isSameRow
                ? PieceBase::getNumChar(color, abs(fromRow - toRow))
                : PieceBase::getColChar(color, isBottom, toCol));

    Q_ASSERT(getSeatPair(zhStr) == seatPair); //验证
    return zhStr;
}

SeatPair Board::getSeatPair(const QString& zhStr) const
{
    SeatPair seatPair;
    if (zhStr.size() != 4)
        return seatPair;

    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color { PieceBase::getColorFromZh(zhStr.back()) };
    bool isBottom { color == bottomColor_ };
    int index {}, movDir { PieceBase::getMovNum(isBottom, zhStr.at(2)) };
    QChar name { zhStr.front() };
    bool isPawn = false;

    QList<Seat*> seats;
    if (PieceBase::isPiece(name)) { // 首字符为棋子名
        seats = boardPieces_->getLiveSeats(color, name,
            PieceBase::getCol(isBottom, PieceBase::getNum(color, zhStr.at(1))));

        //        if (seats.size() == 0)
        //            return seatPair;

        Q_ASSERT(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        isPawn = PieceBase::isPawnPiece(name);
        seats = (isPawn
                ? boardPieces_->getLiveSeats_SortPawn(color, isBottom)
                : boardPieces_->getLiveSeats(color, name));

        //        if (seats.size() <= 1)
        //            return seatPair;

        Q_ASSERT(seats.size() > 1);
        // 如是兵，已根据是否底边排好序
        index = PieceBase::getIndex(seats.size(), isPawn ? false : isBottom, zhStr.front());
    }

    //    if (index >= seats.size())
    //        return seatPair;

    Q_ASSERT(index <= seats.size() - 1);
    if (!isPawn) // 按先行后列的顺序，从小到大排序
        std::sort(seats.begin(), seats.end(), SeatBase::less);

    seatPair.first = seats.at(index);
    int num { PieceBase::getNum(color, zhStr.back()) },
        toCol { PieceBase::getCol(isBottom, num) };
    if (PieceBase::isLinePiece(name)) {
        seatPair.second = (movDir == 0
                ? boardSeats_->getSeat({ seatPair.first->row(), toCol })
                : boardSeats_->getSeat({ seatPair.first->row() + movDir * num, seatPair.first->col() }));
    } else { // 斜线走子：仕、相、马
        int colAway { abs(toCol - seatPair.first->col()) }, //  相距1或2列
            rowInc { PieceBase::isAdvisorBishopPiece(name) ? colAway : (colAway == 1 ? 2 : 1) };
        seatPair.second = boardSeats_->getSeat({ seatPair.first->row() + movDir * rowInc, toCol });
    }

    //    Q_ASSERT(zhStr == getZhStr(seatPair));
    return seatPair;
}

SeatPair Board::getSeatPair(const QPair<Coord, Coord>& coordlPair) const
{
    return { boardSeats_->getSeat(coordlPair.first), boardSeats_->getSeat(coordlPair.second) };
}

QString Board::toString(bool hasEdge) const
{
    return boardSeats_->toString(bottomColor_, hasEdge);
}

QList<QList<Coord>> Board::canMove(Seat* fromSeat) const
{
    if (!fromSeat->hasPiece())
        return {};

    // 1.可移动位置；2.排除规则不允许行走的位置；3.排除同色的位置
    QList<QList<Coord>> allCoords = fromSeat->canMove(boardSeats_, getHomeSide(fromSeat->piece()->color()));

    // 4.排除将帅对面或被将军的位置
    allCoords.append(filterKilledRule(fromSeat, allCoords[0]));

    return allCoords;
}

QList<Coord> Board::filterKilledRule(Seat* fromSeat, QList<Coord>& coords) const
{
    //  排除将帅对面、被将军的位置
    QList<Coord> killCoords;
    QMutableListIterator<Coord> coordIter(coords);
    while (coordIter.hasNext()) {
        Coord& coord = coordIter.next();
        Seat* toSeat = boardSeats_->getSeat(coord);
        // 查询能走的位置时，可能包括对方将帅的位置
        if (toSeat->hasPiece() && toSeat->piece()->kind() == PieceKind::KING)
            continue;

        if (isFaceOrKilled(fromSeat, toSeat)) {
            killCoords.append(coord);
            coordIter.remove();
        }
    }

    return killCoords;
}

bool Board::isFaceOrKilled(Seat* fromSeat, Seat* toSeat) const
{
    PieceColor color = fromSeat->piece()->color();
    Piece* toPiece = toSeat->piece();

    fromSeat->moveTo(toSeat);
    bool result { isFace() || isKilled(color) };
    toSeat->moveTo(fromSeat, toPiece);

    return result;
}

SeatSide Board::getHomeSide(PieceColor color) const
{
    return color == bottomColor_ ? SeatSide::BOTTOM : SeatSide::TOP;
}

bool Board::setBottomColor()
{
    Seat* redKingSeat = boardPieces_->getKingSeat(PieceColor::RED);
    Q_ASSERT(redKingSeat);
    if (!redKingSeat)
        return false;

    bottomColor_ = (redKingSeat->isBottom() ? PieceColor::RED : PieceColor::BLACK);
    return true;
}
