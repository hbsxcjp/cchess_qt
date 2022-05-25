#include "manualmove.h"
#include "board.h"
#include "manualmoveiterator.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "seatbase.h"

#ifdef DEBUG
#include "seat.h"
#include "tools.h"
#endif

ManualMove::ManualMove()
    : rootMove_(new Move)
    , curMove_(rootMove_)
{
}

ManualMove::~ManualMove()
{
    Move::deleteMove(rootMove_);
}

Move* ManualMove::append_coordPair(const Board* board, const CoordPair& coordPair,
    const QString& remark, bool isOther)
{
    return append_seatPair(board, board->getSeatPair(coordPair), remark, isOther);
}

Move* ManualMove::append_rowcols(const Board* board, const QString& rowcols, const QString& remark, bool isOther)
{
    return append_coordPair(board, SeatBase::coordPair(rowcols), remark, isOther);
}

Move* ManualMove::append_iccs(const Board* board, const QString& iccs, const QString& remark, bool isOther)
{
    CoordPair coordPair { { PieceBase::getRowFrom(iccs[1]), PieceBase::getColFrom(iccs[0]) },
        { PieceBase::getRowFrom(iccs[3]), PieceBase::getColFrom(iccs[2]) } };
    return append_coordPair(board, coordPair, remark, isOther);
}

Move* ManualMove::append_zhStr(const Board* board, const QString& zhStr, const QString& remark, bool isOther)
{
    return append_seatPair(board, {}, remark, isOther, zhStr);
}

void ManualMove::setMoveNums()
{
    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
    ManualMoveFirstNextIterator firstNextIter(this);
    while (firstNextIter.hasNext()) {
        Move* move = firstNextIter.next();

        if (move->isOther())
            ++maxCol_;
        ++movCount_;
        maxCol_ = std::max(maxCol_, move->otherIndex());
        maxRow_ = std::max(maxRow_, move->nextIndex());
        move->setCC_ColIndex(maxCol_); // # 本着在视图中的列数
        if (!move->remark().isEmpty()) {
            ++remCount_;
            remLenMax_ = std::max(remLenMax_, move->remark().length());
        }
    }
}

bool ManualMove::backDeleteMove()
{
    if (curMove_->isRoot())
        return false;

    Move* oldCurMove = curMove_;
    bool succes { false };
    if ((succes = backOther()))
        curMove_->setOtherMove(oldCurMove->otherMove());
    else if ((succes = backNext()))
        curMove_->setNextMove(Q_NULLPTR);

    if (succes)
        Move::deleteMove(oldCurMove);

    return succes;
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

bool ManualMove::goNext()
{
    if (!curMove_->hasNext())
        return false;

    curMove_ = curMove_->nextMove();
    curMove_->done();
    return true;
}

bool ManualMove::backNext()
{
    if (!curMove_->isNext())
        return false;

    curMove_->undo();
    curMove_ = curMove_->preMove();
    return true;
}

bool ManualMove::goOther()
{
    if (!curMove_->hasOther())
        return false;

    curMove_->undo();
    curMove_ = curMove_->otherMove();
    curMove_->done();
    return true;
}

bool ManualMove::backOther()
{
    if (!curMove_->isOther())
        return false;

    curMove_->undo(); // 变着回退
    curMove_ = curMove_->preMove();
    curMove_->done(); // 前变执行
    return true;
}

bool ManualMove::goToOther(Move* otherMove)
{
    goNext();

    while (curMove_ != otherMove && curMove_->hasOther())
        goOther();

    return curMove_ == otherMove;
}

bool ManualMove::backAllOtherNext()
{
    while (backOther())
        ;

    return backNext();
}

bool ManualMove::backAllNextOther()
{
    while (backNext())
        ;

    return backOther();
}

bool ManualMove::backNextToHasOther()
{
    while (backNext())
        if (curMove_->hasOther())
            break;

    return curMove_->hasOther();
}

bool ManualMove::goEndPre()
{
    if (!curMove_->hasNext())
        return false;

    bool moved { false };
    while (curMove_->nextMove()->hasNext())
        moved = goNext();

    return moved;
}

bool ManualMove::goOtherEndPre()
{
    if (curMove_->otherMove()->isLeaf())
        return false;

    goOther();
    while (!curMove_->isLeaf()
        && !((curMove_->hasNext() && curMove_->nextMove()->isLeaf())
            || (curMove_->hasOther() && curMove_->otherMove()->isLeaf())))
        if (curMove_->hasNext()) {
            goEndPre();
            if (curMove_->nextMove()->hasOther())
                goNext();
        } else
            goOther();

    return true;
}

bool ManualMove::goEnd()
{
    goEndPre();
    return goNext();
}

bool ManualMove::backStart()
{
    if (curMove_->isRoot())
        return false;

    while (backAllOtherNext())
        ;

    return true;
}

bool ManualMove::goTo(Move* move)
{
    //    if (!move || curMove_ == move)
    if (curMove_ == move)
        return false;

    backStart();
    for (auto& move : move->getPrevMoves())
        move->done();

    curMove_ = move;
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

bool ManualMove::go()
{
    return goIs(!curMove_->hasNext());
}

bool ManualMove::back()
{
    return backIs(curMove_->isOther());
}

bool ManualMove::goInc(int inc)
{
    if (inc == 0 || !curMove_->hasNext())
        return false;

    while (inc-- && goNext())
        ;

    return true;
}

bool ManualMove::backInc(int inc)
{
    if (inc == 0 || curMove_->isRoot())
        return false;

    while (inc-- && backNext())
        ;

    return true;
}

bool ManualMove::curMoveIs(Move* move) const
{
    return curMove_ == move;
}

const QString& ManualMove::getCurRemark() const
{
    return curMove_->remark();
}

void ManualMove::setCurRemark(const QString& remark) const
{
    curMove_->setRemark(remark);
}

SeatPair ManualMove::getCurSeatPair() const
{
    return curMove_->seatPair();
}

CoordPair ManualMove::getCurCoordPair() const
{
    return curMove_->coordPair();
}

QString ManualMove::moveInfo() const
{
    return QString("【着法深度：%1, 视图宽度：%2, 着法数量：%3, 注解数量：%4, 注解最长：%5】\n")
        .arg(maxRow())
        .arg(maxCol())
        .arg(getMovCount())
        .arg(getRemCount())
        .arg(getRemLenMax());
}

QString ManualMove::curZhStr() const
{
    return curMove_->zhStr();
}

Move* ManualMove::append_seatPair(const Board* board, SeatPair seatPair, const QString& remark,
    bool isOther, QString zhStr)
{
    if (isOther)
        curMove_->undo();

    if (zhStr.isEmpty()) {
        zhStr = board->getZhStr(seatPair);
        if (zhStr.isEmpty())
            return Q_NULLPTR;
    } else
        seatPair = board->getSeatPair(zhStr);

    // 疑难文件通不过下面的检验，需注释
    if (!board->isCanMove(seatPair))
        return Q_NULLPTR;

#ifdef DEBUG
    //*// 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (seatPair.first->toString() + seatPair.second->toString()
            + zhStr + (isOther ? " isOther.\n" : "\n")),
        QIODevice::Append);
    //*/

    // 疑难文件通不过下面的检验，需注释此行
    bool canMove = board->isCanMove(seatPair);
    if (!canMove) {
        Tools::writeTxtFile("test.txt",
            QString("失败：\n%1%2%3 %4\n%5\n")
                .arg(seatPair.first->toString())
                .arg(seatPair.second->toString())
                .arg(zhStr)
                .arg(isOther ? "isOther." : "")
                .arg(board->toString()),
            QIODevice::Append);

        //        qDebug() << __FILE__ << __LINE__;
        //        return Q_NULLPTR;
    }
    Q_ASSERT(canMove);
#endif

    if (isOther)
        curMove_->done();

    Move* move = new Move(curMove_, seatPair, zhStr, remark, isOther);
    goIs(isOther);

#ifdef DEBUG
    //*// 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (board->toString() + curMove_->toString() + '\n'),
        QIODevice::Append);
    //*/
#endif

    return move;
}
