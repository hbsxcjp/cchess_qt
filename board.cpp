#include "board.h"
#include "aspect.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"

Board::Board()
    : pieces_(new Pieces())
    , seats_(new Seats())
    , bottomColor_(Color::RED)
{
}

Board::~Board()
{
    delete seats_;
    delete pieces_;
}

void Board::clean()
{
    seats_->clear();
    bottomColor_ = Color::RED;
}

void Board::reinit()
{
    setFEN(Pieces::FENStr);
}

QList<PSeat> Board::getLiveSeatList(Color color) const
{
    return pieces_->getLiveSeatList(color);
}

QList<QList<SeatCoord>> Board::canMove(SeatCoord seatCoord) const
{
    PSeat fromSeat = seats_->getSeat(seatCoord);
    PPiece piece = fromSeat->getPiece();
    Q_ASSERT(piece);

    Color color = piece->color();
    // 1.可移动位置；2.规则已排除位置；3.同色已排除位置
    QList<QList<SeatCoord>> seatCoordLists = piece->canMoveSeatCoord(seats_, getHomeSide_(color));

    // 4.将帅对面或被将军已排除位置
    seatCoordLists.append(killFilterSeatCoord(fromSeat, seatCoordLists[0]));

    return seatCoordLists;
}

QMap<PSeat, QList<SeatCoord>> Board::allCanMove(Color color) const
{
    QMap<PSeat, QList<SeatCoord>> seatCoord_seatCoordList;
    for (auto& seat : getLiveSeatList(color)) {
        auto seatCoordList = canMove(seat->seatCoord()).at(0);
        if (seatCoordList.count() > 0)
            seatCoord_seatCoordList[seat] = seatCoordList;
    }

    return seatCoord_seatCoordList;
}

bool Board::isCanMove(SeatCoord fromSeatCoord, SeatCoord toSeatCoord) const
{
    return canMove(fromSeatCoord).at(0).contains(toSeatCoord);
}

bool Board::isCanMove(const MovSeat& movSeat) const
{
    return isCanMove(movSeat.first->seatCoord(), movSeat.second->seatCoord());
}

bool Board::isFace() const
{
    PSeat redSeat { pieces_->getKingSeat(Color::RED) },
        blackSeat { pieces_->getKingSeat(Color::BLACK) };
    int col { redSeat->col() };
    if (col != blackSeat->col())
        return false;

    int redRow = redSeat->row(), blackRow = blackSeat->row();
    int lowRow { qMin(redRow, blackRow) },
        upRow { qMax(redRow, blackRow) };
    for (int row = lowRow + 1; row < upRow; ++row)
        if (seats_->getSeat(row, col)->getPiece())
            return false;

    return true;
}

bool Board::isKilling(Color color) const
{
    auto kingSeatCoord = pieces_->getKingSeat(color)->seatCoord();
    Color otherColor = Pieces::getOtherColor(color);
    Side otherHomeSide = getHomeSide_(otherColor);
    for (auto& seat : getLiveSeatList(otherColor)) {
        auto seatCoordLists = seat->getPiece()->canMoveSeatCoord(seats_, otherHomeSide);
        if (seatCoordLists.at(0).contains(kingSeatCoord))
            return true;
    }

    return false;
}

bool Board::isFailed(Color color) const
{
    return allCanMove(color).count() == 0;
}

QString Board::getFEN() const
{
    return seats_->getFEN();
}

bool Board::setFEN(const QString& fen)
{
    bool success = seats_->setFEN(pieces_, fen);
    if (success)
        setBottomColor_();

    return success;
}

MovSeat Board::getChangeMovSeat(MovSeat movSeat, ChangeType ct) const
{
    return { seats_->getChangeSeat(movSeat.first, ct), seats_->getChangeSeat(movSeat.second, ct) };
}

void Board::changeLayout(ChangeType ct)
{
    seats_->changeLayout(pieces_, ct);
    setBottomColor_();
}

QString Board::getZhStr(const MovSeat& movSeat) const
{
    QString qstr {};
    PSeat fseat { movSeat.first }, tseat { movSeat.second };
    PPiece fromPiece { fseat->getPiece() };
    Q_ASSERT(fromPiece);

    Color color { fromPiece->color() };
    QChar name { fromPiece->name() };
    int fromRow { fseat->row() }, fromCol { fseat->col() },
        toRow { tseat->row() }, toCol { tseat->col() };
    bool isSameRow { fromRow == toRow }, isBottom { color == bottomColor_ };
    auto seatList = pieces_->getLiveSeatList(color, name, fromCol);

    if (seatList.size() > 1 && pieces_->isKindName(name, Pieces::strongeKindList)) {
        if (pieces_->isKindName(name, { Kind::PAWN }))
            seatList = pieces_->getSortPawnLiveSeatList(color, isBottom);
        qstr.append(Pieces::getIndexChar(seatList.size(), isBottom, seatList.indexOf(fseat)))
            .append(name);
    } else { //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        qstr.append(name)
            .append(Pieces::getColChar(color, isBottom, fromCol));
    }
    qstr.append(Pieces::getMovChar(isSameRow, isBottom, toRow > fromRow))
        .append(pieces_->isKindName(name, Pieces::lineKindList) && !isSameRow
                ? Pieces::getNumChar(color, abs(fromRow - toRow))
                : Pieces::getColChar(color, isBottom, toCol));

    Q_ASSERT(getMovSeat(qstr) == movSeat); //验证
    return qstr;
}

MovSeat Board::getMovSeat(const QString& zhStr, bool ignoreError) const
{
    Q_ASSERT(zhStr.size() == 4);
    MovSeat movSeat;
    QList<PSeat> seats;

    // 根据最后一个字符判断该着法属于哪一方
    Color color { Pieces::getColorFromZh(zhStr.back()) };
    bool isBottom { color == bottomColor_ };
    int index {}, movDir { Pieces::getMovNum(isBottom, zhStr.at(2)) };
    QChar name { zhStr.front() };

    if (pieces_->isKindName(name, Pieces::allKindList)) { // 首字符为棋子名
        seats = pieces_->getLiveSeatList(color, name,
            Pieces::getCol(isBottom, Pieces::getNum(color, zhStr.at(1))));

        if (ignoreError && seats.size() == 0)
            return movSeat;

        Q_ASSERT(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        seats = (pieces_->isKindName(name, { Kind::PAWN })
                ? pieces_->getSortPawnLiveSeatList(color, isBottom)
                : pieces_->getLiveSeatList(color, name));

        if (ignoreError && seats.size() == 0)
            return movSeat;

        Q_ASSERT(seats.size() > 0);
        index = Pieces::getIndex(seats.size(), isBottom, zhStr.front());
    }

    Q_ASSERT(index <= seats.length() - 1);
    movSeat.first = seats.at(index);
    int num { Pieces::getNum(color, zhStr.back()) },
        toCol { Pieces::getCol(isBottom, num) };
    if (pieces_->isKindName(name, Pieces::lineKindList)) {
        movSeat.second = (movDir == 0
                ? seats_->getSeat(movSeat.first->row(), toCol)
                : seats_->getSeat(movSeat.first->row() + movDir * num, movSeat.first->col()));
    } else { // 斜线走子：仕、相、马
        int colAway { abs(toCol - movSeat.first->col()) }, //  相距1或2列
            rowInc { pieces_->isKindName(name, { Kind::ADVISOR, Kind::BISHOP }) ? colAway : (colAway == 1 ? 2 : 1) };
        movSeat.second = seats_->getSeat(movSeat.first->row() + movDir * rowInc, toCol);
    }

    //    Q_ASSERT(zhStr == getZhStr(movSeat));
    return movSeat;
}

MovSeat Board::getMovSeat(int rowcols) const
{
    return getMovSeat(Seats::seatCoordPair(rowcols));
}

MovSeat Board::getMovSeat(QPair<SeatCoord, SeatCoord> seatCoordlPair) const
{
    return { seats_->getSeat(seatCoordlPair.first), seats_->getSeat(seatCoordlPair.second) };
}

QString Board::getZhChars() const
{
    return pieces_->getZhChars();
}

QString Board::toString(bool full) const
{
    // 棋盘上下边标识字符串
    const QString PRESTR[] = {
        "　　　　　　　黑　方　　　　　　　\n１　２　３　４　５　６　７　８　９\n",
        "　　　　　　　红　方　　　　　　　\n一　二　三　四　五　六　七　八　九\n"
    };
    const QString SUFSTR[] = {
        "九　八　七　六　五　四　三　二　一\n　　　　　　　红　方　　　　　　　\n",
        "９　８　７　６　５　４　３　２　１\n　　　　　　　黑　方　　　　　　　\n"
    };

    auto textBlankBoard = seats_->toString();
    if (!full)
        return textBlankBoard;

    int index = int(bottomColor_);
    return PRESTR[index] + textBlankBoard + SUFSTR[index];
}

PAspectStatus Board::getAspectStatus(Color color) const
{
    return new AspectStatus(0, isKilling(color),
        false /*willKill(color)*/, false /*isCatch(color)*/, isFailed(color));
}

QList<SeatCoord> Board::killFilterSeatCoord(PSeat fromSeat, QList<SeatCoord>& seatCoordList) const
{
    //  排除将帅对面、被将军的位置
    QList<SeatCoord> killSeatCoord;
    QMutableListIterator<SeatCoord> seatCoordIter(seatCoordList);
    Color color = fromSeat->getPiece()->color();
    while (seatCoordIter.hasNext()) {
        auto& toSeatCoord = seatCoordIter.next();
        auto toSeat = seats_->getSeat(toSeatCoord);
        auto toPiece = fromSeat->moveTo(toSeat);
        if (isFace() || isKilling(color)) {
            killSeatCoord.append(toSeatCoord);
            seatCoordIter.remove();
        }
        toSeat->moveTo(fromSeat, toPiece);
    }

    return killSeatCoord;
}

Side Board::getHomeSide_(Color color) const
{
    return color == bottomColor_ ? Side::HERE : Side::THERE;
}

void Board::setBottomColor_()
{
    bottomColor_ = (pieces_->getKingSeat(Color::RED)->row() < SEATROW / 2
            ? Color::RED
            : Color::BLACK);
}
