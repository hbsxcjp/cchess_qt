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
        getSeat(coordPiece.first)->setPiece(boardPieces_->getNonLivePiece(color, kind));
    }

    bottomColor_ = PieceColor::RED;
}

QList<Piece*> Board::getAllPieces() const
{
    return boardPieces_->getAllPieces();
}

QList<Seat*> Board::getLiveSeats() const
{
    return boardPieces_->getLiveSeats();
}

// test.cpp
Piece* Board::getPiece(Coord coord) const
{
    return getSeat(coord)->piece();
}

QList<Coord> Board::getLiveSeatCoordList(PieceColor color) const
{
    return getCoordList(boardPieces_->getLiveSeats(color));
}

QList<Coord> Board::getCanMoveCoords(const Coord& fromCoord) const
{
    return getCanMoveCoordLists(fromCoord).value(0);
}

QList<QList<Coord>> Board::getCanMoveCoordLists(const Coord& fromCoord) const
{
    return getCanMoveCoordLists(getSeat(fromCoord));
}

QMap<Seat*, QList<Coord>> Board::getColorCanMoveCoords(PieceColor color) const
{
    QMap<Seat*, QList<Coord>> seatCoords;
    for (auto& fromSeat : boardPieces_->getLiveSeats(color))
        seatCoords[fromSeat] = getCanMoveCoords(fromSeat->coord());

    return seatCoords;
}
// test.cpp end.

// bool Board::canPut(Piece* piece, const Coord& coord) const
//{
//     return SeatBase::canPut(piece->kind(), getHomeSide(piece->color())).contains(coord);
// }

void Board::placePiece(Piece* piece, const Coord& coord) const
{
    getSeat(coord)->setPiece(piece);
}

Piece* Board::takeOutPiece(const Coord& coord) const
{
    Seat* seat = getSeat(coord);
    Piece* piece = seat->piece();
    seat->setPiece(Q_NULLPTR);

    return piece;
}

bool Board::canMove(const Coord& fromCoord, const Coord& toCoord) const
{
    return canMove({ getSeat(fromCoord), getSeat(toCoord) });
}

bool Board::canMove(const SeatPair& seatPair) const
{
    return getCanMoveCoords(seatPair.first->coord()).contains(seatPair.second->coord());
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
    for (auto& fromSeat : boardPieces_->getLiveSeats(otherColor))
        if (fromSeat->canMove(boardSeats_, otherHomeSide).value(0).contains(kingCoord))
            return true;

    return false;
}

bool Board::isFailed(PieceColor color) const
{
    for (auto& fromSeat : boardPieces_->getLiveSeats(color))
        if (!getCanMoveCoords(fromSeat->coord()).isEmpty())
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

SeatPair Board::changeSeatPair(SeatPair seatPair, ChangeType ct) const
{
    return { boardSeats_->changeSeat(seatPair.first, ct),
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
            // ?????????????????????????????????????????????
            std::sort(seats.begin(), seats.end(), SeatBase::less);

        // ??????????????????????????????????????????
        bool tempIsBottom { isPawn ? false : isBottom };
        QChar indexChar { PieceBase::getIndexChar(seats.size(), tempIsBottom, seats.indexOf(fseat)) };
        zhStr.append(indexChar).append(name);
    } else { //??????, ???(???),???(???): ???????????????????????????????????????????????????????????????????????????????????????
        QChar colChar { PieceBase::getColChar(color, isBottom, fromCol) };
        zhStr.append(name).append(colChar);
    }
    bool lineNotSameRow { PieceBase::isLinePiece(name) && !isSameRow };
    QChar movChar { PieceBase::getMovChar(isSameRow, isBottom, toRow > fromRow) },
        toNumColChar { lineNotSameRow
                ? PieceBase::getNumChar(color, abs(fromRow - toRow))
                : PieceBase::getColChar(color, isBottom, toCol) };
    zhStr.append(movChar).append(toNumColChar);

    Q_ASSERT(getSeatPair(zhStr) == seatPair); //??????
    return zhStr;
}

SeatPair Board::getSeatPair(const QString& zhStr) const
{
    SeatPair seatPair;
    if (zhStr.size() != 4)
        return seatPair;

    // ??????????????????????????????????????????????????????
    PieceColor color { PieceBase::getColorFromZh(zhStr.back()) };
    bool isBottom { color == bottomColor_ };
    int index {}, movDir { PieceBase::getMovNum(isBottom, zhStr.at(2)) };
    QChar name { zhStr.front() };
    bool isMultPawn = false;

    QList<Seat*> seats;
    if (PieceBase::isPiece(name)) { // ?????????????????????
        int fromNum { PieceBase::getNum(color, zhStr.at(1)) },
            fromCol { PieceBase::getCol(isBottom, fromNum) };
        seats = boardPieces_->getLiveSeats(color, name, fromCol);

        //        if (seats.size() == 0)
        //            return seatPair;

        Q_ASSERT(seats.size() > 0);
        //# ???????????????????????????????????????????????????????????????????????????????????????????????????index
        bool advBishopBack { seats.size() == 2 && movDir == -1 };
        index = advBishopBack ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        isMultPawn = PieceBase::isPawnPiece(name);
        seats = (isMultPawn
                ? boardPieces_->getLiveSeats_SortPawn(color, isBottom)
                : boardPieces_->getLiveSeats(color, name));

        //        if (seats.size() <= 1)
        //            return seatPair;

        Q_ASSERT(seats.size() > 1);
        // ?????????????????????????????????????????????
        index = PieceBase::getIndex(seats.size(), isMultPawn ? false : isBottom, zhStr.front());
    }

    //    if (index >= seats.size())
    //        return seatPair;

    Q_ASSERT(index <= seats.size() - 1);
    if (!isMultPawn) // ???????????????????????????????????????????????????????????????
        std::sort(seats.begin(), seats.end(), SeatBase::less);

    seatPair.first = seats.at(index);
    int toNum { PieceBase::getNum(color, zhStr.back()) },
        toRow { seatPair.first->row() },
        toCol { PieceBase::getCol(isBottom, toNum) };
    if (PieceBase::isLinePiece(name)) {
        if (movDir != 0) {
            toRow += movDir * toNum;
            toCol = seatPair.first->col();
        }
    } else { // ??????????????????????????????
        int colAway { abs(toCol - seatPair.first->col()) }, //  ??????1???2???
            rowInc { PieceBase::isAdvisorBishopPiece(name) ? colAway : (colAway == 1 ? 2 : 1) };
        toRow += movDir * rowInc;
    }
    seatPair.second = getSeat({ toRow, toCol });

    //    Q_ASSERT(zhStr == getZhStr(seatPair));
    return seatPair;
}

SeatPair Board::getSeatPair(const QPair<Coord, Coord>& coordlPair) const
{
    return { getSeat(coordlPair.first), getSeat(coordlPair.second) };
}

QString Board::toString(bool hasEdge) const
{
    return boardSeats_->toString(bottomColor_, hasEdge);
}

Seat* Board::getSeat(const Coord& coord) const
{
    return boardSeats_->getSeat(coord);
}

QList<QList<Coord>> Board::getCanMoveCoordLists(Seat* fromSeat) const
{
    if (!fromSeat->hasPiece())
        return {};

    // 1.??????????????????2.???????????????????????????????????????3.?????????????????????
    QList<QList<Coord>> allCoords = fromSeat->canMove(boardSeats_, getHomeSide(fromSeat->piece()->color()));

    // 4.???????????????????????????????????????
    allCoords.append(filterKilledRule(fromSeat, allCoords[0]));

    return allCoords;
}

QList<Coord> Board::filterKilledRule(Seat* fromSeat, QList<Coord>& coords) const
{
    //  ???????????????????????????????????????
    QList<Coord> killCoords;
    QMutableListIterator<Coord> coordIter(coords);
    while (coordIter.hasNext()) {
        Coord& coord = coordIter.next();
        Seat* toSeat = getSeat(coord);
        // ????????????????????????????????????????????????????????????
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
