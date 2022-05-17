#include "manualmoveiterator.h"
#include "board.h"
#include "manualmove.h"
#include "move.h"
#include "piecebase.h"
#include "seatbase.h"

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

ManualMoveReverseIterator::ManualMoveReverseIterator(ManualMove* aManualMove)
    : ManualMoveIterator(aManualMove)
    , otherMoves(QList<Move*>())
{
}

bool ManualMoveReverseIterator::checkBehind()
{
    // 遍历已回到根节点
    if (manualMove->move()->isRoot())
        return false;

    if (manualMove->move()->hasOther()) {
        // 前检查着是next
        if (!isOther) {
            bool moved = manualMove->goOtherEndPre();
            isOther = moved ? !manualMove->move()->hasNext() : true;
        } else
            isOther = manualMove->move()->isOther();
    } else
        isOther = false;

    return true;
}

void ManualMoveReverseIterator::afterUsed()
{
    if (!firstCheck) {
        Move* move = manualMove->move();
        if (isOther)
            move->done();

        // 连续多个Other时，首次遇到不移动，后次遇到应后退
        QPair<int, int> nextOtherIndex { move->nextIndex(), move->otherIndex() };
        if (move->isOther() && move->hasOther() && !otherMoves.contains(move))
            otherMoves.append(move);
        else
            manualMove->back();
    } else {
        manualMove->goEndPre();
        firstCheck = false;
    }
}

ManualMoveAppendableIterator::ManualMoveAppendableIterator(ManualMove* aManualMove)
    : isOther(false)
    , otherMoves(QList<Move*>())
    , manualMove(aManualMove)
{
}

ManualMoveAppendableIterator::~ManualMoveAppendableIterator()
{
    manualMove->backStart();
    manualMove->setMoveNums();
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const CoordPair& coordPair,
    const QString& remark, bool hasNext, bool hasOther)
{
    return goAppendMove(board, board->getSeatPair(coordPair), remark, hasNext, hasOther);
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const QString& rowcols,
    const QString& remark, bool hasNext, bool hasOther)
{
    return goAppendMove(board, SeatBase::coordPair(rowcols), remark, hasNext, hasOther);
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const QString& iccsOrZhStr,
    const QString& remark, bool isPGN_ZH, bool hasNext, bool hasOther)
{
    if (!isPGN_ZH) {
        CoordPair coordPair { { PieceBase::getRowFrom(iccsOrZhStr[1]), PieceBase::getColFrom(iccsOrZhStr[0]) },
            { PieceBase::getRowFrom(iccsOrZhStr[3]), PieceBase::getColFrom(iccsOrZhStr[2]) } };
        return goAppendMove(board, coordPair, remark, hasNext, hasOther);
    }

    if (isOther)
        manualMove->move()->undo();

    SeatPair seatPair = board->getSeatPair(iccsOrZhStr);
    if (isOther)
        manualMove->move()->done();

    return goAppendMove(board, seatPair, remark, hasNext, hasOther);
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const QString& zhStr)
{
    return goAppendMove(board, zhStr, "", true, true, false);
}

bool ManualMoveAppendableIterator::backDeleteMove()
{
    return manualMove->backDeleteMove();
}

Move* ManualMoveAppendableIterator::goAppendMove(const Board* board, const SeatPair& seatPair,
    const QString& remark, bool hasNext, bool hasOther)
{
    Move* move = manualMove->goAppendMove(board, seatPair, remark, isOther);
    isOther = !hasNext;
    handleOtherPreMove(move, hasNext, hasOther);

    return move;
}

void ManualMoveAppendableIterator::handleOtherPreMove(Move* move, bool hasNext, bool hasOther)
{
    if (hasNext && hasOther)
        otherMoves.append(move);

    if (hasNext || hasOther)
        return;

    if (otherMoves.isEmpty())
        return;

    Move* preMove = otherMoves.takeLast();
    while (manualMove->move() != preMove)
        manualMove->back();
}
