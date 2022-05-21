#include "manualmoveiterator.h"
#include "manual.h"
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

ManualMoveAppendIterator::ManualMoveAppendIterator(Manual* manual)
    : isOther_(false)
    , board(manual->board())
    , preMoves({})
    , manualMove(manual->manualMove())
{
    preMoves.push(manual->manualMove()->move());
}

ManualMoveAppendIterator::~ManualMoveAppendIterator()
{
    manualMove->backStart();
    manualMove->setMoveNums();
}

bool ManualMoveAppendIterator::isEnd() const
{
    return manualMove->move()->isRoot() && manualMove->move()->hasNext(); // 已返回至根节点
}

Move* ManualMoveAppendIterator::appendGo(const CoordPair& coordPair,
    const QString& remark, bool hasNext, bool hasOther)
{
    Move* move = manualMove->appendGo(board, coordPair, remark, isOther_);
    handlePreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendIterator::appendGo(const QString& rowcols,
    const QString& remark, bool hasNext, bool hasOther)
{
    Move* move = manualMove->appendGo(board, rowcols, remark, isOther_);
    handlePreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendIterator::appendGo(const QString& iccsOrZhStr,
    const QString& remark, bool isPGN_ZH, bool hasNext, bool hasOther)
{
    Move* move = manualMove->appendGo(board, iccsOrZhStr, remark, isPGN_ZH, isOther_);
    handlePreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendIterator::appendGo(const QString& iccsOrZhStr,
    const QString& remark, bool isPGN_ZH, int endBranchNum, bool hasOther)
{
    Move* move = manualMove->appendGo(board, iccsOrZhStr, remark, isPGN_ZH, isOther_);
    handlePreMove(move, endBranchNum, hasOther);

    return move;
}

Move* ManualMoveAppendIterator::appendGo(const QString& zhStr)
{
    Move* move = manualMove->appendGo(board, zhStr);
    handlePreMove(move, false, 0, false);

    return move;
}

bool ManualMoveAppendIterator::backDeleteMove()
{
    return manualMove->backDeleteMove();
}

void ManualMoveAppendIterator::handlePreMove(Move* move, bool hasNext, bool hasOther)
{
    bool isOther { !hasNext }, hasBranch { hasNext && hasOther };
    int endBranchNum { (hasNext || hasOther) ? 0 : 1 };
    handlePreMove(move, isOther, endBranchNum, hasBranch);
}

void ManualMoveAppendIterator::handlePreMove(Move* move, int endBranchNum, bool hasOther)
{
    bool isOther { hasOther }, hasBranch { hasOther };
    handlePreMove(move, isOther, endBranchNum, hasBranch);
}

void ManualMoveAppendIterator::handlePreMove(Move* move, bool isOther, int endBranchNum, bool hasBranch)
{
    isOther_ = isOther;
    // 暂存分支
    if (hasBranch)
        preMoves.push(move);

    // 后退至分支
    Move* preMove {};
    while (endBranchNum-- && !preMoves.isEmpty())
        preMove = preMoves.pop();

    if (preMove)
        manualMove->goTo(preMove);
}
