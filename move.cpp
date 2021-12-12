#include "move.h"
#include "board.h"
#include "piece.h"
#include "seat.h"

Move::Move()
{
}

Move::Move(PMove preMove, MovSeat movSeat, const QString& zhStr, const QString& remark)
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

int Move::rowcols() const
{
    return Seats::rowcols(movSeat_.first->rowcol(), movSeat_.second->rowcol());
}

void Move::done()
{
    movSeat_.first->moveTo(movSeat_.second);
}

void Move::undo()
{
    movSeat_.second->moveTo(movSeat_.first, toPiece_);
}

bool Move::isOther()
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
    if (this->preMove()) {
        PMove move { this };
        moveList.append(move);
        while ((move = move->getPrevMove()))
            if (move->preMove()) // 排除根着法
                moveList.prepend(move);
    }

    return moveList;
}

void Move::changeLayout(const PBoard& board, ChangeType ct)
{
    movSeat_ = board->getChangeMovSeat(movSeat_, ct);
    toPiece_ = movSeat_.second->getPiece();
    zhStr_ = board->getZhStr(movSeat_);
}

QString Move::iccs() const
{
    PSeat fseat { movSeat_.first }, tseat { movSeat_.second };
    return ((fseat && tseat)
            ? (Pieces::getColICCSChar(fseat->col()) + QString::number(fseat->row())
                + Pieces::getColICCSChar(tseat->col()) + QString::number(tseat->row()))
            : QString {});
}

QString Move::toString() const
{
    PSeat fseat { movSeat_.first }, tseat { movSeat_.second };
    if (!fseat || !tseat)
        return "rootMove.\n";

    QString qstr {};
    QTextStream stream(&qstr);
    stream << fseat->toString() << '_' << tseat->toString()
           << '=' << iccs() << '=' << zhStr_ << '{' << remark_ << "}\n";

    return qstr;
}
