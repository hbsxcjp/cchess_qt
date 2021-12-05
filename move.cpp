#include "move.h"
#include "seat.h"

Move::Move()
{
}

Move::Move(PMove preMove, MovSeat movSeat)
    : movSeat_(movSeat)
    , toPiece_(movSeat.second->getPiece())
    , preMove_(preMove)
{
}

PMove Move::addMove(MovSeat movSeat, bool isOther)
{
    PMove move = new Move(this, movSeat);
    isOther ? setOtherMove(move) : setNextMove(move);

    return move;
}

void Move::deleteMove(PMove move)
{
    if (!move)
        return;

    PMove nextMove { nextMove_ }, otherMove { otherMove_ };
    delete move;

    deleteMove(nextMove);
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
