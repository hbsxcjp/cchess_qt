#include "move.h"
#include "board.h"
#include "boardpieces.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"

const QString rootZhStr { "开始" };

Move::Move(Move* preMove, const SeatPair& seatPair, const QString& zhStr,
    const QString& remark, bool isOther)
    : fromSeat_(seatPair.first)
    , toSeat_(seatPair.second)
    , toPiece_(Q_NULLPTR)
    //    , preMove_(preMove)
    //    , nextMove_(Q_NULLPTR)
    //    , otherMove_(Q_NULLPTR)
    , zhStr_(zhStr)
    , remark_(remark)
{
    //    if (isOther) {
    //        nextIndex_ = preMove->nextIndex();
    //        otherIndex_ = preMove->otherIndex() + 1;
    //        preMove->setOtherMove(this);
    //        preMove->otherMove_ = this;
    //    } else {
    //        nextIndex_ = preMove->nextIndex() + 1;
    //        otherIndex_ = preMove->otherIndex();
    //        preMove->setNextMove(this);
    //        preMove->nextMove_ = this;
    //    }
    (isOther ? preMove->setOtherMove(this) : preMove->setNextMove(this));
    setNextIndex();
    setOtherIndex();
}

void Move::deleteMove(Move* move)
{
    if (!move)
        return;

    Move* nextMove { move->nextMove() };
    Move* otherMove { move->otherMove() };
    delete move;

    deleteMove(nextMove);

    deleteMove(otherMove);
}

PieceColor Move::color() const
{
    return fromSeat_->piece()->color();
}

PieceColor Move::color_done() const
{
    return toSeat_->piece()->color();
}

void Move::setNextMove(Move* move)
{
    nextMove_ = move;
    if (move)
        move->preMove_ = this;
}

void Move::setOtherMove(Move* move)
{
    otherMove_ = move;
    if (move)
        move->preMove_ = this;
}

void Move::setNextIndex()
{
    nextIndex_ = preMove_->nextIndex() + (isNext() ? 1 : 0);
}

void Move::setOtherIndex()
{
    otherIndex_ = preMove_->otherIndex() + (isOther() ? 1 : 0);
}

CoordPair Move::coordPair() const
{
    if (isRoot())
        return {};

    return { fromSeat_->coord(), toSeat_->coord() };
}

QString Move::rowcols() const
{
    if (isRoot())
        return {};

    return QString("%1%2%3%4")
        .arg(fromSeat_->row())
        .arg(fromSeat_->col())
        .arg(toSeat_->row())
        .arg(toSeat_->col());
}

QString Move::iccs() const
{
    if (isRoot())
        return {};

    return QString("%1%2%3%4")
        .arg(PieceBase::getIccsChar(fromSeat_->col()))
        .arg(fromSeat_->row())
        .arg(PieceBase::getIccsChar(toSeat_->col()))
        .arg(toSeat_->row());
}

void Move::done()
{
    toPiece_ = toSeat_->piece();
    fromSeat_->moveTo(toSeat_);
}

void Move::undo()
{
    toSeat_->moveTo(fromSeat_, toPiece_);
}

bool Move::isNext() const
{
    return preMove_ && preMove_->nextMove() == this;
}

bool Move::isOther() const
{
    return preMove_ && preMove_->otherMove() == this;
}

QList<Move*> Move::getPrevMoves()
{
    if (isRoot())
        return {};

    auto getPrevMove_ = [](Move*& move) {
        while (move->isOther())
            move = move->preMove();

        return (move = move->preMove());
    };

    Move* move { this };
    QList<Move*> moves { move };
    while (!getPrevMove_(move)->isRoot())
        moves.prepend(move);

    return moves;
}

bool Move::changeLayout(const Board* board, ChangeType ct)
{
    SeatPair seats = board->changeSeatPair(seatPair(), ct);
    fromSeat_ = seats.first;
    toSeat_ = seats.second;
    toPiece_ = toSeat_->piece();
    zhStr_ = board->getZhStr(seatPair());

    return !zhStr_.isEmpty();
}

QString Move::toString() const
{
    if (!fromSeat_ || !toSeat_)
        return "rootMove.\n";

    return QString("%1_%2=%3=%4{%5}\n")
        .arg(fromSeat_->toString())
        .arg(toSeat_->toString())
        .arg(iccs())
        .arg(zhStr_)
        .arg(remark_);
}
