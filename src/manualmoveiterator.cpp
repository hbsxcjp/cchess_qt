#include "manualmoveiterator.h"
#include "manualmove.h"
#include "move.h"

ManualMoveIterator::ManualMoveIterator(ManualMove* aManualMove)
    : oldCurMove(aManualMove->move())
    , manualMove(aManualMove)
{
    reset();
}

void ManualMoveIterator::reset()
{
    manualMove->backStart();
    isOther = false;
    firstCheck = true;
}

bool ManualMoveIterator::hasNext()
{
    afterUsed();
    bool has = checkBehind();
    beforeUse(has);

    return has;
}

Move*& ManualMoveIterator::next() const
{
    return isOther ? manualMove->move()->otherMove() : manualMove->move()->nextMove();
}

void ManualMoveIterator::beforeUse(bool has)
{
    if (!has)
        manualMove->goTo(oldCurMove);
    else if (isOther)
        manualMove->move()->undo();
}

void ManualMoveIterator::afterUsed()
{
    if (!firstCheck) {
        if (isOther)
            manualMove->move()->done();

        manualMove->goIs(isOther);
    } else
        firstCheck = false;
}

bool ManualMoveOnlyNextIterator::checkBehind()
{
    return manualMove->move()->hasNext();
}

bool ManualMoveFirstNextIterator::checkBehind()
{
    bool has { false };
    if ((has = manualMove->move()->hasNext())) {
        isOther = false;
    } else {
        while (!(has = manualMove->move()->hasOther()))
            if (!manualMove->backAllOtherNext())
                break;

        isOther = true;
    }

    return has;
}

bool ManualMoveFirstOtherIterator::checkBehind()
{
    bool has { false };
    if ((has = manualMove->move()->hasOther())) {
        isOther = true;
    } else {
        while (!(has = manualMove->move()->hasNext()))
            if (!manualMove->backAllNextOther())
                break;

        isOther = false;
    }

    return has;
}

// 待修改
bool ManualMoveReverseIterator::checkBehind()
{
    bool has { false };
    if ((has = manualMove->move()->hasNext())) {
        isOther = false;
    } else {
        while (!(has = manualMove->move()->hasOther()))
            if (!manualMove->backAllOtherNext())
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
    return manualMove->goAppendMove(board, seatPair, remark, isOther);
}

bool ManualMoveMutableIterator::backDeleteMove()
{
    return manualMove->backDeleteMove();
}
