#include "move.h"
#include "board.h"
#include "boardpieces.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"

Move* Move::creatRootMove()
{
    return new Move;
}

Move* Move::creatMove(Move* preMove, const Board* board, const SeatPair& seatPair,
    const QString& remark, bool isOther)
{
    if (isOther)
        preMove->undo();

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
        preMove->done();

    Move* move = new Move(preMove, seatPair, zhStr, remark, isOther);
    //    go(isOther);

#ifdef DEBUG
    /*// 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (board_->toString() + curMove_->toString() + '\n'),
        QIODevice::Append);
    //*/
#endif

    return move;
}

Move::Move(Move* preMove, const SeatPair& seatPair, const QString& zhStr,
    const QString& remark, bool isOther)
    : seatPair_(seatPair)
    , toPiece_(seatPair.second->piece())
    , preMove_(preMove)
    , nextMove_(Q_NULLPTR)
    , otherMove_(Q_NULLPTR)
    , zhStr_(zhStr)
    , remark_(remark)
{
    if (isOther) {
        nextIndex_ = preMove->nextIndex();
        otherIndex_ = preMove->otherIndex() + 1;
        preMove->setOtherMove(this);
    } else {
        nextIndex_ = preMove->nextIndex() + 1;
        otherIndex_ = preMove->otherIndex();
        preMove->setNextMove(this);
    }
}

void Move::deleteMove(Move* move)
{
    Move* nextMove { move->nextMove() };
    Move* otherMove { move->otherMove() };
    delete move;

    if (nextMove)
        deleteMove(nextMove);

    if (otherMove)
        deleteMove(otherMove);
}

PieceColor Move::color() const
{
    return seatPair_.first->piece()->color();
}

CoordPair Move::coordPair() const
{
    if (isRoot())
        return {};

    return { seatPair_.first->coord(), seatPair_.second->coord() };
}

QString Move::rowcols() const
{
    if (isRoot())
        return {};

    Seat* fseat { seatPair_.first };
    Seat* tseat { seatPair_.second };
    return QString("%1%2%3%4").arg(fseat->row()).arg(fseat->col()).arg(tseat->row()).arg(tseat->col());
}

QString Move::iccs() const
{
    if (isRoot())
        return {};

    Seat* fseat { seatPair_.first };
    Seat* tseat { seatPair_.second };
    return QString("%1%2%3%4")
        .arg(PieceBase::getIccsChar(fseat->col()))
        .arg(fseat->row())
        .arg(PieceBase::getIccsChar(tseat->col()))
        .arg(tseat->row());
}

void Move::done() const
{
    seatPair_.first->moveTo(seatPair_.second);
}

void Move::undo() const
{
    seatPair_.second->moveTo(seatPair_.first, toPiece_);
}

bool Move::isNext() const
{
    return preMove_ && preMove_->nextMove() == this;
}

bool Move::isOther() const
{
    return preMove_ && preMove_->otherMove() == this;
}

Move* Move::getPrevMove() const
{
    const Move* move { this };
    while (move->isOther())
        move = move->preMove();

    return move->preMove();
}

QList<const Move*> Move::getPrevMoves() const
{
    QList<const Move*> moves {};
    if (isRoot())
        return moves;

    const Move* move { this };
    moves.append(move);
    while ((move = move->getPrevMove()))
        if (!move->isRoot())
            moves.prepend(move);

    return moves;
}

bool Move::changeLayout(const Board* board, ChangeType ct)
{
    board->changeSeatPair(seatPair_, ct);
    toPiece_ = seatPair_.second->piece();
    zhStr_ = board->getZhStr(seatPair_);

    return !zhStr_.isEmpty();
}

QString Move::toString() const
{
    Seat* fseat { seatPair_.first };
    Seat* tseat { seatPair_.second };
    if (!fseat || !tseat)
        return "rootMove.\n";

    return QString("%1_%2=%3=%4{%5}\n")
        .arg(fseat->toString())
        .arg(tseat->toString())
        .arg(iccs())
        .arg(zhStr_)
        .arg(remark_);
}
