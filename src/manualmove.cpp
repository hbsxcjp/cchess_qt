#include "manualmove.h"
#include "board.h"
#include "move.h"
#include "piece.h"

ManualMove::ManualMove()
    : rootMove_(new Move)
    , curMove(rootMove_)
{
}

ManualMove::~ManualMove()
{
    Move::deleteMove(rootMove_);
}

Move* ManualMove::appendMove(const Board* board, const SeatPair& seatPair, const QString& remark, bool isOther)
{
    if (isOther)
        curMove->undo();

    // 疑难文件通不过下面的检验，需注释
    if (!board->isCanMove(seatPair))
        return Q_NULLPTR;

    QString zhStr { board->getZhStr(seatPair) };
    if (zhStr.isEmpty())
        return Q_NULLPTR;

#ifdef DEBUG
    /*// 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (seatPair.first->toString() + seatPair.second->toString()
            + zhStr + (isOther ? " isOther.\n" : "\n")),
        QIODevice::Append);
    //*/

    // 疑难文件通不过下面的检验，需注释此行
    bool canMove = board_->isCanMove(seatPair);
    if (!canMove) {
        Tools::writeTxtFile("test.txt",
            QString("失败：\n%1%2%3 %4\n%5\n")
                .arg(seatPair.first->toString())
                .arg(seatPair.second->toString())
                .arg(zhStr)
                .arg(isOther ? "isOther." : "")
                .arg(board_->toString()),
            QIODevice::Append);

        //        qDebug() << __FILE__ << __LINE__;
        //        return Q_NULLPTR;
    }
    Q_ASSERT(canMove);
#endif

    if (isOther)
        curMove->done();

    Move* move = new Move(curMove, seatPair, zhStr, remark, isOther);
    goIs(isOther);

#ifdef DEBUG
    /*// 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (board_->toString() + curMove_->toString() + '\n'),
        QIODevice::Append);
    //*/
#endif

    return move;
}

bool ManualMove::isEmpty() const
{
    return !rootMove_->hasNext();
}

PieceColor ManualMove::firstColor() const
{
    if (isEmpty())
        return PieceColor::RED;

    return rootMove_->nextMove()->color();
}

void ManualMove::setCurMove(Move*& move)
{
    curMove = move;
    curMove->done();
}

bool ManualMove::goNext()
{
    if (!curMove->hasNext())
        return false;

    curMove = curMove->nextMove();
    curMove->done();
    return true;
}

bool ManualMove::backNext()
{
    if (!curMove->isNext())
        return false;

    curMove->undo();
    curMove = curMove->preMove();
    return true;
}

bool ManualMove::goOther()
{
    if (!curMove->hasOther())
        return false;

    curMove->undo();
    curMove = curMove->otherMove();
    curMove->done();
    return true;
}

bool ManualMove::backOther()
{
    if (!curMove->isOther())
        return false;

    curMove->undo(); // 变着回退
    curMove = curMove->preMove();
    curMove->done(); // 前变执行
    return true;
}

bool ManualMove::goToOther(Move* otherMove)
{
    goNext();

    while (curMove != otherMove && otherMove->isOther())
        goOther();

    return curMove == otherMove;
}

bool ManualMove::backToPre()
{
    while (backOther())
        ;

    return backNext();
}

bool ManualMove::goEnd()
{
    if (!curMove->hasNext())
        return false;

    while (goNext())
        ;

    return true;
}

bool ManualMove::backStart()
{
    if (curMove->isRoot())
        return false;

    while (backToPre())
        ;

    return true;
}

bool ManualMove::goTo(Move* move)
{
    if (!move || curMove == move)
        return false;

    backStart();
    for (auto& move : move->getPrevMoves())
        move->done();

    curMove = move;
    return true;
}

bool ManualMove::goIs(bool isOther)
{
    return isOther ? goOther() : goNext();
}

bool ManualMove::backIs(bool isOther)
{
    return isOther ? backOther() : backNext();
}

bool ManualMove::goInc(int inc)
{
    if (inc == 0 || !curMove->hasNext())
        return false;

    while (inc-- && goNext())
        ;

    return true;
}

bool ManualMove::backInc(int inc)
{
    if (inc == 0 || curMove->isRoot())
        return false;

    while (inc-- && backNext())
        ;

    return true;
}

QString ManualMove::toString() const
{
    return curMove->zhStr();
}
