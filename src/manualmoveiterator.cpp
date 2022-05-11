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

Move*& ManualMoveIterator::doneNext() const
{
    return manualMove->move();
}

void ManualMoveIterator::curMoveDone() const
{
    if (!manualMove->move()->isRoot())
        manualMove->move()->done();
}

void ManualMoveIterator::curMoveUndo(bool has) const
{
    if (has)
        manualMove->move()->undo();
}

void ManualMoveIterator::checkFinished(bool has) const
{
    if (!has)
        manualMove->goTo(oldCurMove);
}

bool ManualMoveOnlyNextIterator::hasNext()
{
    curMoveDone();
    bool has = checkGoNext();
    curMoveUndo(has);

    return has;
}

bool ManualMoveOnlyNextIterator::hasDoneNext()
{
    return checkGoNext();
}

bool ManualMoveOnlyNextIterator::checkGoNext() const
{
    bool has = manualMove->goNext();
    checkFinished(has);

    return has;
}

bool ManualMoveFirstNextIterator::hasNext()
{
    //    curMoveDone();
    //    bool has = checkGoNextOther();
    //    curMoveUndo(has);

    //    return has;

    static bool firstCheck { true };
    if (!firstCheck) {
        firstCheck = false;
        if (isOther)
            manualMove->move()->done();

        manualMove->goIs(isOther);
    }

    bool has = checkNextOther();

    if (has && isOther)
        manualMove->move()->undo();

    return has;
}

bool ManualMoveFirstNextIterator::hasDoneNext()
{
    return checkGoNextOther();
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
    checkFinished(has);

    return has;
}

bool ManualMoveFirstNextIterator::checkGoNextOther() const
{
    bool has { false };
    if (!(has = manualMove->goNext())) {
        while (!(has = manualMove->goOther()))
            if (!manualMove->backToPre())
                break;
    }
    checkFinished(has);

    return has;
}
