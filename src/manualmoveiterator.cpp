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
    isOther = false;
    firstCheck = true;
    manualMove->backStart();
}

bool ManualMoveIterator::hasNext()
{
    beforeCheck();
    bool has = checkBehind();
    afterCheck(has);

    return has;
}

Move*& ManualMoveIterator::next() const
{
    return isOther ? manualMove->move()->otherMove() : manualMove->move()->nextMove();
}

void ManualMoveIterator::beforeCheck()
{
    if (firstCheck) {
        firstCheck = false;
        return;
    }

    if (isOther) {
        manualMove->move()->done();
        manualMove->goOther();
    } else
        manualMove->goNext();
}

void ManualMoveIterator::afterCheck(bool has)
{
    if (!has)
        manualMove->goTo(oldCurMove);
    else if (isOther)
        manualMove->move()->undo();
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

ManualMoveAppendIterator::ManualMoveAppendIterator(ManualMove* manualMove)
    : isOther_(false)
    , preMoves_({})
    , manualMove_(manualMove)
{
    preMoves_.push(manualMove->move());
}

ManualMoveAppendIterator::~ManualMoveAppendIterator()
{
    manualMove_->backStart();
    manualMove_->setNumValues();
}

bool ManualMoveAppendIterator::isEnd() const
{
    return manualMove_->move()->isRoot() && manualMove_->move()->hasNext(); // 已返回至根节点
}

Move* ManualMoveAppendIterator::append_coordPair(const CoordPair& coordPair,
    const QString& remark, bool hasNext, bool hasOther)
{
    Move* move = manualMove_->append_coordPair(coordPair, remark);
    firstNextHandlePreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendIterator::append_rowcols(const QString& rowcols,
    const QString& remark, bool hasNext, bool hasOther)
{
    Move* move = manualMove_->append_rowcols(rowcols, remark);
    firstNextHandlePreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendIterator::append_zhStr(const QString& zhStr, const QString& remark, bool hasNext, bool hasOther)
{
    Move* move = manualMove_->append_zhStr(zhStr, remark);
    firstNextHandlePreMove(move, hasNext, hasOther);

    return move;
}

Move* ManualMoveAppendIterator::append_iccsZhStr(const QString& iccsOrZhStr,
    const QString& remark, bool isPGN_ZH, int endBranchNum, bool hasOther)
{
    Move* move = (isPGN_ZH
            ? manualMove_->append_zhStr(iccsOrZhStr, remark)
            : manualMove_->append_iccs(iccsOrZhStr, remark));
    firstOtherHandlePreMove(move, endBranchNum, hasOther);

    return move;
}

void ManualMoveAppendIterator::firstNextHandlePreMove(Move* move, bool hasNext, bool hasOther)
{
    isOther_ = !hasNext;

    bool hasBranch { hasNext && hasOther };
    int endBranchNum = !(hasNext || hasOther);
    backBranchNum(move, hasBranch, endBranchNum);
}

void ManualMoveAppendIterator::firstOtherHandlePreMove(Move* move, int endBranchNum, bool hasOther)
{
    isOther_ = hasOther;

    bool hasBranch { hasOther };
    backBranchNum(move, hasBranch, endBranchNum);
}

void ManualMoveAppendIterator::backBranchNum(Move* move, bool hasBranch, int endBranchNum)
{
    if (hasBranch)
        preMoves_.push(move);

    if (endBranchNum == 0)
        return;

    Move* preMove {};
    while (endBranchNum-- && !preMoves_.isEmpty())
        preMove = preMoves_.pop();

    while (!manualMove_->isCurMove(preMove))
        manualMove_->back();
}
