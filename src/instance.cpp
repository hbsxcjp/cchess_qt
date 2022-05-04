#include "instance.h"
#include "aspect.h"
#include "board.h"
#include "boardpieces.h"
#include "boardseats.h"
#include "instanceio.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"
#include "tools.h"

Instance::Instance()
    : board_(new Board)
    , rootMove_(Move::creatRootMove())
    , curMove_(rootMove_)
    , info_(InfoMap())
    , status_(InsStatus::MOVEDEMO)
{
}

Instance::~Instance()
{
    Move::deleteMove(rootMove_); // 驱动函数
    delete board_;
}

void Instance::reset()
{
    Move::deleteMove(rootMove_); // 驱动函数
    rootMove_ = Move::creatRootMove();
    curMove_ = rootMove_;

    board_->init();
}

QList<Piece*> Instance::getAllPiece() const
{
    return board_->getAllPiece();
}

QList<Seat*> Instance::getLiveSeats() const
{
    return board_->getLiveSeats();
}

Move* Instance::appendMove(const CoordPair& coordPair, const QString& remark, bool isOther)
{
    return appendMove(board_->getSeatPair(coordPair), remark, isOther);
}

Move* Instance::appendMove(const QString& iccsOrZhStr, const QString& remark, bool isOther, bool isPGN_ZH)
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

Move* Instance::appendMove(const QString& rowcols, const QString& remark, bool isOther)
{
    return appendMove(SeatBase::coordPair(rowcols), remark, isOther);
}

Move* Instance::appendMove(const QString& zhStr)
{
    //    SeatPair seatPair = board_->getSeatPair(zhStr);
    //    if (!seatPair.first || !board_->isCanMove(seatPair))
    //        return Q_NULLPTR;

    //    Move* move = new Move(curMove_, seatPair, zhStr, "", false);
    //    goNext();

    //    return move;
    return appendMove(zhStr, "", false, true);
}

bool Instance::go(bool isOther)
{
    return isOther ? goOther() : goNext();
}

bool Instance::goNext()
{
    if (!curMove_->nextMove())
        return false;

    curMove_ = curMove_->nextMove();
    curMove_->done();
    return true;
}

bool Instance::goOther()
{
    if (!curMove_->otherMove())
        return false;

    curMove_->undo();
    curMove_ = curMove_->otherMove();
    curMove_->done();
    return true;
}

void Instance::goEnd()
{
    while (goNext())
        ;
}

void Instance::goTo(Move* move)
{
    if (!move || curMove_ == move)
        return;

    backStart();
    for (auto& move : move->getPrevMoves())
        move->done();

    curMove_ = move;
}

bool Instance::back(bool isOther)
{
    return isOther ? backOther() : backNext();
}

bool Instance::backOne()
{
    return curMove_->isOther() ? backOther() : backNext();
}

bool Instance::backNext()
{
    if (!curMove_->isNext())
        return false;

    curMove_->undo();
    curMove_ = curMove_->preMove();
    return true;
}

bool Instance::backOther()
{
    if (!curMove_->isOther())
        return false;

    curMove_->undo(); // 变着回退
    curMove_ = curMove_->preMove();
    curMove_->done(); // 前变执行
    return true;
}

bool Instance::backToPre()
{
    while (backOther())
        ;

    return backNext();
}

void Instance::backStart()
{
    while (backToPre())
        ;
}

void Instance::backTo(Move* move)
{
    while (!isStart() && curMove_ != move)
        backOne();
}

void Instance::goOrBackInc(int inc)
{
    int incCount { abs(inc) };
    // std::function<void(Instance*)> fbward = inc > 0 ? &Instance::go : &Instance::back;
    auto fbward = std::mem_fn(inc > 0 ? &Instance::goNext : &Instance::backNext);
    while (incCount-- && fbward(this))
        ;
}

bool Instance::changeLayout(ChangeType ct)
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

QString Instance::getInfoValue(InfoIndex nameIndex)
{
    return info_[InstanceIO::getInfoName(nameIndex)];
}

void Instance::setInfoValue(InfoIndex nameIndex, const QString& value)
{
    info_[InstanceIO::getInfoName(nameIndex)] = value;
}

bool Instance::isStart() const
{
    return curMove_->isRoot();
}

bool Instance::isEnd() const
{
    return !curMove_->nextMove();
}

bool Instance::hasOther() const
{
    return curMove_->otherMove();
}

bool Instance::isOther() const
{
    return curMove_->isOther();
}

QString Instance::getECCORowcols() const
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

void Instance::setEcco(const QStringList& eccoRec)
{
    info_[InstanceIO::getInfoName(InfoIndex::ECCOSN)] = eccoRec.at(0);
    info_[InstanceIO::getInfoName(InfoIndex::ECCONAME)] = eccoRec.at(1);
}

SeatPair Instance::getCurSeatPair() const
{
    return curMove_->seatPair();
}

CoordPair Instance::getCurCoordPair() const
{
    return curMove_->coordPair();
}

QList<Coord> Instance::canPut(Piece* piece) const
{
    return SeatBase::canPut(piece->kind(), getHomeSide(piece->color()));
}

QList<Coord> Instance::canMove(const Coord& coord) const
{
    return board_->canMove(coord).value(0);
}

void Instance::setMoveNums()
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

void Instance::setFEN(const QString& fen, PieceColor color)
{
    info_["FEN"] = QString("%1 %2 - - 0 1").arg(fen).arg((color == PieceColor::RED ? "r" : "b"));
}

void Instance::setBoard()
{
    const QString& fen = info_["FEN"];
    board_->setFEN(fen.left(fen.indexOf(' ')));

    //    qDebug() << getPieceChars();
}

SeatSide Instance::getHomeSide(PieceColor color) const
{
    return board_->getHomeSide(color);
}

QString Instance::getPieceChars() const
{
    return board_->getPieceChars();
}

QString Instance::moveInfo() const
{
    return QString("【着法深度：%1, 视图宽度：%2, 着法数量：%3, 注解数量：%4, 注解最长：%5】\n")
        .arg(maxRow_)
        .arg(maxCol_)
        .arg(movCount_)
        .arg(remCount_)
        .arg(remLenMax_);
}

QString Instance::boardString(bool full) const
{
    return curMove_->toString() + board_->toString(full);
}

QString Instance::toMoveString(StoreType storeType) const
{
    return InstanceIO::getMoveString(this, storeType);
}

QString Instance::toString(StoreType storeType) const
{
    return InstanceIO::getString(this, storeType);
}

QString Instance::toFullString()
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

QList<Aspect> Instance::getAspectList()
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

Move* Instance::appendMove(const SeatPair& seatPair, const QString& remark, bool isOther)
{
    Move* move = Move::creatMove(curMove_, board_, seatPair, remark, isOther);
    if (move)
        go(isOther);

    return move;
}
