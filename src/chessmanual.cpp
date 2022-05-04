#include "chessmanual.h"
#include "aspect.h"
#include "board.h"
#include "boardpieces.h"
#include "boardseats.h"
#include "chessmanualIO.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"
#include "tools.h"

ChessManual::ChessManual()
    : board_(new Board)
    , rootMove_(Move::creatRootMove())
    , curMove_(rootMove_)
    , info_(InfoMap())
    , status_(ManualStatus::MOVEDEMO)
{
}

ChessManual::~ChessManual()
{
    Move::deleteMove(rootMove_); // 驱动函数
    delete board_;
}

void ChessManual::reset()
{
    Move::deleteMove(rootMove_); // 驱动函数
    rootMove_ = Move::creatRootMove();
    curMove_ = rootMove_;

    board_->init();
}

QList<Piece*> ChessManual::getAllPiece() const
{
    return board_->getAllPiece();
}

QList<Seat*> ChessManual::getLiveSeats() const
{
    return board_->getLiveSeats();
}

Move* ChessManual::appendMove(const CoordPair& coordPair, const QString& remark, bool isOther)
{
    return appendMove(board_->getSeatPair(coordPair), remark, isOther);
}

Move* ChessManual::appendMove(const QString& iccsOrZhStr, const QString& remark, bool isOther, bool isPGN_ZH)
{
    if (!isPGN_ZH) {
        CoordPair coordPair { { PieceBase::getRowFrom(iccsOrZhStr[1]), PieceBase::getColFrom(iccsOrZhStr[0]) },
            { PieceBase::getRowFrom(iccsOrZhStr[3]), PieceBase::getColFrom(iccsOrZhStr[2]) } };
        return appendMove(coordPair, remark, isOther);
    }

    if (isOther)
        curMove_->undo();

    SeatPair seatPair = board_->getSeatPair(iccsOrZhStr);
    if (isOther)
        curMove_->done();

    return appendMove(seatPair, remark, isOther);
}

Move* ChessManual::appendMove(const QString& rowcols, const QString& remark, bool isOther)
{
    return appendMove(SeatBase::coordPair(rowcols), remark, isOther);
}

Move* ChessManual::appendMove(const QString& zhStr)
{
    //    SeatPair seatPair = board_->getSeatPair(zhStr);
    //    if (!seatPair.first || !board_->isCanMove(seatPair))
    //        return Q_NULLPTR;

    //    Move* move = new Move(curMove_, seatPair, zhStr, "", false);
    //    goNext();

    //    return move;
    return appendMove(zhStr, "", false, true);
}

bool ChessManual::go(bool isOther)
{
    return isOther ? goOther() : goNext();
}

bool ChessManual::goNext()
{
    if (!curMove_->nextMove())
        return false;

    curMove_ = curMove_->nextMove();
    curMove_->done();
    return true;
}

bool ChessManual::goOther()
{
    if (!curMove_->otherMove())
        return false;

    curMove_->undo();
    curMove_ = curMove_->otherMove();
    curMove_->done();
    return true;
}

void ChessManual::goEnd()
{
    while (goNext())
        ;
}

void ChessManual::goTo(Move* move)
{
    if (!move || curMove_ == move)
        return;

    backStart();
    for (auto& move : move->getPrevMoves())
        move->done();

    curMove_ = move;
}

bool ChessManual::back(bool isOther)
{
    return isOther ? backOther() : backNext();
}

bool ChessManual::backOne()
{
    return curMove_->isOther() ? backOther() : backNext();
}

bool ChessManual::backNext()
{
    if (!curMove_->isNext())
        return false;

    curMove_->undo();
    curMove_ = curMove_->preMove();
    return true;
}

bool ChessManual::backOther()
{
    if (!curMove_->isOther())
        return false;

    curMove_->undo(); // 变着回退
    curMove_ = curMove_->preMove();
    curMove_->done(); // 前变执行
    return true;
}

bool ChessManual::backToPre()
{
    while (backOther())
        ;

    return backNext();
}

void ChessManual::backStart()
{
    while (backToPre())
        ;
}

void ChessManual::backTo(Move* move)
{
    while (!isStart() && curMove_ != move)
        backOne();
}

void ChessManual::goOrBackInc(int inc)
{
    int incCount { abs(inc) };
    // std::function<void(Instance*)> fbward = inc > 0 ? &Instance::go : &Instance::back;
    auto fbward = std::mem_fn(inc > 0 ? &ChessManual::goNext : &ChessManual::backNext);
    while (incCount-- && fbward(this))
        ;
}

bool ChessManual::changeLayout(ChangeType ct)
{
    std::function<bool(bool)>
        changeMove__ = [&](bool isOther) -> bool {
        Move* move = isOther ? curMove_->otherMove() : curMove_->nextMove();

        if (isOther)
            curMove_->undo();
        if (!move->changeLayout(board_, ct))
            return false;
        if (isOther)
            curMove_->done();

        go(isOther);
        if (move->nextMove())
            if (!changeMove__(false))
                return false;

        if (move->otherMove())
            if (!changeMove__(true))
                return false;

        back(isOther);

        return true;
    };

    Move* curMove { curMove_ };
    backStart();
    if (!board_->changeLayout(ct))
        return false;

    if (rootMove_->nextMove())
        if (!changeMove__(false))
            return false;

    backStart();
    Move* firstMove { rootMove_->nextMove() };
    setFEN(board_->getFEN(), firstMove ? firstMove->color() : PieceColor::RED);

    goTo(curMove);

    return true;
}

QString ChessManual::getInfoValue(InfoIndex nameIndex)
{
    return info_[ChessManualIO::getInfoName(nameIndex)];
}

void ChessManual::setInfoValue(InfoIndex nameIndex, const QString& value)
{
    info_[ChessManualIO::getInfoName(nameIndex)] = value;
}

bool ChessManual::isStart() const
{
    return curMove_->isRoot();
}

bool ChessManual::isEnd() const
{
    return !curMove_->nextMove();
}

bool ChessManual::hasOther() const
{
    return curMove_->otherMove();
}

bool ChessManual::isOther() const
{
    return curMove_->isOther();
}

QString ChessManual::getECCORowcols() const
{
    std::function<QString(CoordPair&, ChangeType)>
        getChangeRowcol_ = [](CoordPair& seatCoordPair, ChangeType ct) -> QString {
        seatCoordPair = { SeatBase::changeCoord(seatCoordPair.first, ct),
            SeatBase::changeCoord(seatCoordPair.second, ct) };
        return QString("%1%2%3%4")
            .arg(seatCoordPair.first.first)
            .arg(seatCoordPair.first.second)
            .arg(seatCoordPair.second.first)
            .arg(seatCoordPair.second.second);
    };

    QString allRowcols;
    int color = 0;
    Move* move = rootMove_;
    QString rowcol[4][PieceBase::ALLCOLORS.size()];
    while ((move = move->nextMove())) {
        rowcol[0][color].append(move->rowcols());
        int chIndex = 1;
        CoordPair seatCoordPair = move->coordPair();
        for (ChangeType ct : { ChangeType::SYMMETRY_H, ChangeType::ROTATE, ChangeType::SYMMETRY_H })
            rowcol[chIndex++][color].append(getChangeRowcol_(seatCoordPair, ct));

        color = (color + 1) % 2;
    }

    for (int chIndex = 0; chIndex < 4; ++chIndex)
        allRowcols.append(QString("~%1-%2").arg(rowcol[chIndex][0]).arg(rowcol[chIndex][1]));

    return allRowcols;
}

void ChessManual::setEcco(const QStringList& eccoRec)
{
    info_[ChessManualIO::getInfoName(InfoIndex::ECCOSN)] = eccoRec.at(0);
    info_[ChessManualIO::getInfoName(InfoIndex::ECCONAME)] = eccoRec.at(1);
}

SeatPair ChessManual::getCurSeatPair() const
{
    return curMove_->seatPair();
}

CoordPair ChessManual::getCurCoordPair() const
{
    return curMove_->coordPair();
}

QList<Coord> ChessManual::canPut(Piece* piece) const
{
    return SeatBase::canPut(piece->kind(), getHomeSide(piece->color()));
}

QList<Coord> ChessManual::canMove(const Coord& coord) const
{
    return board_->canMove(coord).value(0);
}

void ChessManual::setMoveNums()
{
    backStart();
    std::function<void(Move*)>
        __setNums = [&](Move* move) {
            ++movCount_;
            maxCol_ = std::max(maxCol_, move->otherIndex());
            maxRow_ = std::max(maxRow_, move->nextIndex());
            move->setCC_ColIndex(maxCol_); // # 本着在视图中的列数
            if (!move->remark().isEmpty()) {
                ++remCount_;
                remLenMax_ = std::max(remLenMax_, move->remark().length());
            }

            move->done();
            if (move->nextMove())
                __setNums(move->nextMove());
            move->undo();

            if (move->otherMove()) {
                ++maxCol_;
                __setNums(move->otherMove());
            }
        };

    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
    if (rootMove_->nextMove())
        __setNums(rootMove_->nextMove()); // 驱动函数
}

void ChessManual::setFEN(const QString& fen, PieceColor color)
{
    info_["FEN"] = QString("%1 %2 - - 0 1").arg(fen).arg((color == PieceColor::RED ? "r" : "b"));
}

void ChessManual::setBoard()
{
    const QString& fen = info_["FEN"];
    board_->setFEN(fen.left(fen.indexOf(' ')));

    //    qDebug() << getPieceChars();
}

SeatSide ChessManual::getHomeSide(PieceColor color) const
{
    return board_->getHomeSide(color);
}

QString ChessManual::getPieceChars() const
{
    return board_->getPieceChars();
}

QString ChessManual::moveInfo() const
{
    return QString("【着法深度：%1, 视图宽度：%2, 着法数量：%3, 注解数量：%4, 注解最长：%5】\n")
        .arg(maxRow_)
        .arg(maxCol_)
        .arg(movCount_)
        .arg(remCount_)
        .arg(remLenMax_);
}

QString ChessManual::boardString(bool full) const
{
    return curMove_->toString() + board_->toString(full);
}

QString ChessManual::toMoveString(StoreType storeType) const
{
    return ChessManualIO::getMoveString(this, storeType);
}

QString ChessManual::toString(StoreType storeType) const
{
    return ChessManualIO::getString(this, storeType);
}

QString ChessManual::toFullString()
{
    QString qstr {};
    QTextStream stream(&qstr);
    stream << toString(StoreType::PGN_CC) << QString("\n着法描述与棋盘布局：\n");

    std::function<void(Move*, bool)>
        __printMoveBoard = [&](Move* move, bool isOther) {
            stream << move->toString();
            QStringList boardString0 = board_->toString().split('\n');

            go(isOther);
            QStringList boardString1 = board_->toString().split('\n');
            for (int i = 0; i < boardString0.count() - 1; ++i) // 最后有个回车符是空行
                stream << boardString0.at(i) + "  " + boardString1.at(i) + '\n';
            stream << move->toString() << "\n\n";

            if (move->nextMove())
                __printMoveBoard(move->nextMove(), false);

            if (move->otherMove())
                __printMoveBoard(move->otherMove(), true);

            back(isOther);
        };

    Move* curMove { curMove_ };
    backStart();
    if (rootMove_->nextMove())
        __printMoveBoard(rootMove_->nextMove(), false);

    goTo(curMove);
    return qstr;
}

QList<Aspect> ChessManual::getAspectList()
{
    QList<Aspect> aspectList {};
    Move* curMove = curMove_;
    backStart();
    std::function<void(Move*)>
        appendAspect__ = [&](Move* move) {
            // 待补充棋局状态变量的取值
            aspectList.append(Aspect(board_->getFEN(), move->color(), move->rowcols()));

            move->done();
            if (move->nextMove())
                appendAspect__(move->nextMove());
            move->undo();

            if (move->otherMove())
                appendAspect__(move->otherMove());
        };

    if (rootMove_->nextMove())
        appendAspect__(rootMove_->nextMove()); // 驱动函数

    goTo(curMove);
    return aspectList;
}

Move* ChessManual::appendMove(const SeatPair& seatPair, const QString& remark, bool isOther)
{
    Move* move = Move::creatMove(curMove_, board_, seatPair, remark, isOther);
    if (move)
        go(isOther);

    return move;
}
