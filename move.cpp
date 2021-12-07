#include "move.h"
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

PMove Move::appendMove(const MovSeat& movSeat, const QString& zhStr, const QString& remark, bool isOther)
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
    PMove nextMove { nextMove_ }, otherMove { otherMove_ };
    delete move;

    if (nextMove)
        deleteMove(nextMove);

    if (otherMove)
        deleteMove(otherMove);
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
    while (move->preMove() && (move->preMove()->otherMove() == move))
        move = move->preMove();

    return move->preMove();
}

QList<PMove> Move::getPrevMoves()
{
    QList<PMove> moves {};
    if (this->preMove()) {
        PMove move { this };
        moves.append(move);
        while ((move = move->getPrevMove()))
            moves.prepend(move);
    }

    return moves;
}

void Move::changeSide(ChangeType ct)
{
    //     auto& movseat = movseatPiece_.first;
    //     changeSeat(movseat.first, ct);
    //     changeSeat(movseat.second, ct);
    //     zhStr_ = board->movSeatToStr(movseat);

    //    //    done();
    //    if (next_)
    //        next_->changeSide(board, ct);
    //    //    undo();

    //    if (other_)
    //        other_->changeSide(board, ct);
}

QString Move::iccs() const
{
    PSeat fseat { movSeat_.first }, tseat { movSeat_.second };
    return ((fseat != tseat)
            ? (Pieces::getColICCSChar(fseat->col()) + QString::number(fseat->row())
                + Pieces::getColICCSChar(tseat->col()) + QString::number(tseat->row()))
            : QString {});
}

QString Move::toString() const
{
    QString qstr {};
    PSeat fseat { movSeat_.first }, tseat { movSeat_.second };
    if (fseat != tseat) {
        QTextStream stream(&qstr);
        stream << fseat->toString() << '_' << tseat->toString()
               << '-' << iccs() << ':' << zhStr_ << '{' << remark_ << "}\n";
    }
    return qstr;
}
