#include "manualmoveiterator.h"
#include "manualmove.h"
#include "move.h"

ManualMoveIterator::ManualMoveIterator(ManualMove* aManualMove)
    : oldCurMove(aManualMove->move())
    , manualMove(aManualMove)
{
    manualMove->backStart();
}

Move*& ManualMoveIterator::next() const
{
    return isOther ? manualMove->move()->otherMove() : manualMove->move()->nextMove();
}

bool ManualMoveOnlyNextIterator::hasNext()
{
    if (!manualMove->move()->isRoot())
        manualMove->goNext();
    return manualMove->move()->hasNext();
}

bool ManualMoveFirstNextIterator::hasNext()
{
    if (!firstCheck) {
        if (isOther)
            manualMove->move()->done();

        manualMove->goIs(isOther);
    } else
        firstCheck = false;

    bool has = checkNextOther();

    if (!has)
        manualMove->goTo(oldCurMove);
    else if (isOther)
        manualMove->move()->undo();

    return has;
}

bool ManualMoveFirstNextIterator::checkNextOther()
{
    bool has { false };
    if ((has = manualMove->move()->hasNext())) {
        isOther = false;
    } else {
        while (!(has = manualMove->move()->hasOther()))
            if (!manualMove->backToPre())
                break;

        isOther = true;
    }

    return has;
}

ManualMoveMutableIterator::ManualMoveMutableIterator(ManualMove* aManualMove)
    : manualMove(aManualMove)
{
}

bool ManualMoveMutableIterator::appendMove(const Board* board, const SeatPair& seatPair,
    const QString& remark, bool isOther)
{
    return manualMove->appendMove(board, seatPair, remark, isOther);
}
