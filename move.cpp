#include "move.h"
#include "board.h"
#include "piece.h"
#include "seat.h"
//#include <QTextStream>

Move::Move(PMove preMove, const MovSeat& movSeat, const QString& zhStr, const QString& remark)
    : movSeat_(movSeat)
    , toPiece_(movSeat.second->getPiece())
    , preMove_(preMove)
    , zhStr_(zhStr)
    , remark_(remark)
{
}

PMove Move::addMove(const MovSeat& movSeat, const QString& zhStr, const QString& remark, bool isOther)
{
    PMove move = new Move(this, movSeat, zhStr, remark);

    if (isOther) {
        move->nextNo_ = nextNo_;
        move->otherNo_ = otherNo_ + 1;
        otherMove_ = move;
    } else {
        move->nextNo_ = nextNo_ + 1;
        move->otherNo_ = otherNo_;
        nextMove_ = move;
    }

    return move;
}

void Move::deleteMove(PMove move)
{
    PMove nextMove { move->nextMove() }, otherMove { move->otherMove() };
    delete move;

    if (nextMove)
        deleteMove(nextMove);

    if (otherMove)
        deleteMove(otherMove);
}

Color Move::color()
{
    return movSeat_.first->getPiece()->color();
}

SeatCoordPair Move::seatCoordPair() const
{
    PSeat fseat { movSeat_.first }, tseat { movSeat_.second };
    if (!fseat || !tseat)
        return { { 0, 0 }, { 0, 0 } };

    return { fseat->seatCoord(), tseat->seatCoord() };
}

QString Move::rowcols() const
{
    PSeat fseat { movSeat_.first }, tseat { movSeat_.second };
    if (!fseat || !tseat)
        return QString {};

    return Seats::rowcols(movSeat_.first->rowcol(), movSeat_.second->rowcol());
}

QString Move::iccs() const
{
    PSeat fseat { movSeat_.first }, tseat { movSeat_.second };
    if (!fseat || !tseat)
        return QString {};

    return QString("%1%2%3%4")
        .arg(Pieces::getIccsChar(fseat->col()))
        .arg(fseat->row())
        .arg(Pieces::getIccsChar(tseat->col()))
        .arg(tseat->row());
}

void Move::done()
{
    toPiece_ = movSeat_.first->moveTo(movSeat_.second);
}

void Move::undo()
{
    movSeat_.second->moveTo(movSeat_.first, toPiece_);
}

bool Move::isOther() const
{
    return preMove_ && preMove_->otherMove() == this;
}

PMove Move::getPrevMove()
{
    PMove move { this };
    while (move->isOther())
        move = move->preMove();

    return move->preMove();
}

QList<PMove> Move::getPrevMoveList()
{
    QList<PMove> moveList {};
    if (!this->preMove()) // 根着法
        return moveList;

    PMove move { this };
    moveList.append(move);
    while ((move = move->getPrevMove()))
        if (move->preMove()) // 非根着法
            moveList.prepend(move);

    return moveList;
}

bool Move::changeLayout(const PBoard& board, ChangeType ct)
{
    movSeat_ = board->getChangeMovSeat(movSeat_, ct);
    toPiece_ = movSeat_.second->getPiece();
    zhStr_ = board->getZhStr(movSeat_);

    return !zhStr_.isEmpty();
}

QString Move::toString() const
{
    PSeat fseat { movSeat_.first }, tseat { movSeat_.second };
    if (!fseat || !tseat)
        return "rootMove.\n";

    return QString("%1_%2=%3=%4{%5}\n")
        .arg(fseat->toString())
        .arg(tseat->toString())
        .arg(iccs())
        .arg(zhStr_)
        .arg(remark_);
}
