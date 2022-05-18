#include "manualmoveiterator.h"
#include "manualmove.h"
#include "move.h"

#include <QDebug>

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

ManualMoveAppendableIterator::ManualMoveAppendableIterator(ManualMove* aManualMove)
    : isOther(false)
    , otherMoves({ aManualMove->move() })
    , manualMove(aManualMove)
{
}

ManualMoveAppendableIterator::~ManualMoveAppendableIterator()
{
    manualMove->setMoveNums();
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const CoordPair& coordPair,
    const QString& remark, bool hasNext, bool hasOther)
{
    Move* move = manualMove->goAppendMove(board, coordPair, remark, isOther);
    handleOtherPreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const QString& rowcols,
    const QString& remark, bool hasNext, bool hasOther)
{
    Move* move = manualMove->goAppendMove(board, rowcols, remark, isOther);
    handleOtherPreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const QString& iccsOrZhStr,
    const QString& remark, bool isPGN_ZH, bool hasNext, bool hasOther)
{
    Move* move = manualMove->goAppendMove(board, iccsOrZhStr, remark, isPGN_ZH, isOther);
    handleOtherPreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const QString& zhStr)
{
    Move* move = manualMove->goAppendMove(board, zhStr);
    handleOtherPreMove(move, true, false);

    return move;
}

bool ManualMoveAppendableIterator::backDeleteMove()
{
    return manualMove->backDeleteMove();
}

void ManualMoveAppendableIterator::handleOtherPreMove(Move* move, bool hasNext, bool hasOther)
{
    isOther = !hasNext;
    // 暂存Other分支
    if (hasNext && hasOther)
        otherMoves.append(move);

    // 正常前进
    if (hasNext || hasOther)
        return;

    // 后退至Other分支
    Move* preMove = otherMoves.takeLast();
    while (manualMove->move() != preMove)
        manualMove->back();
}
