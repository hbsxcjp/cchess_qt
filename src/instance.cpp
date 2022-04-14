#include "instance.h"
#include "aspect.h"
#include "board.h"
#include "instanceio.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"

Instance::Instance()
    : board_(new Board)
    , rootMove_(new Move)
    , curMove_(rootMove_)
    , info_(InstanceIO::getInitInfoMap())
    , status_(InsStatus::MOVEDEMO)
{
    board_->initFEN();
}

Instance::~Instance()
{
    Move::deleteMove(rootMove_); // 驱动函数
    delete board_;
}

void Instance::reset()
{
    Move::deleteMove(rootMove_); // 驱动函数
    rootMove_ = new Move;
    curMove_ = rootMove_;
    info_ = InstanceIO::getInitInfoMap();

    board_->initFEN();
}

PMove Instance::appendMove(const MovSeat& movSeat, const QString& remark, bool isOther)
{
    if (isOther)
        curMove_->undo();

    if (!movSeat.first || !movSeat.first->getPiece())
        return Q_NULLPTR;

    QString zhStr { board_->getZhStr(movSeat) };
    if (zhStr.isEmpty())
        return Q_NULLPTR;

#ifdef DEBUG
    /*// 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (movSeat.first->toString() + movSeat.second->toString()
            + zhStr + (isOther ? " isOther.\n" : "\n")),
        QIODevice::Append);
    //*/

    if (!board_->isCanMove(movSeat)) {
        Tools::writeTxtFile("test.txt",
            QString("失败：\n%1%2%3 %4\n%5\n")
                .arg(movSeat.first->toString())
                .arg(movSeat.second->toString())
                .arg(zhStr)
                .arg(isOther ? "isOther." : "")
                .arg(board_->toString()),
            QIODevice::Append);
        return Q_NULLPTR;
    }

    // 疑难文件通不过下面的检验，需注释此行
    Q_ASSERT(board_->isCanMove(movSeat));
#endif

    if (isOther)
        curMove_->done();

    curMove_->addMove(movSeat, zhStr, remark, isOther);
    go(isOther);

#ifdef DEBUG
    /*// 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (board_->toString() + curMove_->toString() + '\n'),
        QIODevice::Append);
    //*/
#endif

    return curMove_;
}

PMove Instance::appendMove(SeatCoordPair seatCoordlPair, const QString& remark, bool isOther)
{
    return appendMove(board_->getMovSeat(seatCoordlPair), remark, isOther);
}

PMove Instance::appendMove(QList<QChar> iccs, const QString& remark, bool isOther)
{
    return appendMove(SeatCoordPair { { Pieces::getRowFrom(iccs[1]), Pieces::getColFrom(iccs[0]) },
                          { Pieces::getRowFrom(iccs[3]), Pieces::getColFrom(iccs[2]) } },
        remark, isOther);
}

PMove Instance::appendMove(const QString& zhStr, const QString& remark, bool isOther)
{
    if (isOther)
        curMove_->undo();

    MovSeat movseat = board_->getMovSeat(zhStr);
    if (isOther)
        curMove_->done();

    return appendMove(movseat, remark, isOther);
}

PMove Instance::appendMove_rowcols(const QString& rowcols, const QString& remark, bool isOther)
{
    return appendMove(board_->getMovSeat_rowcols(rowcols), remark, isOther);
}

PMove Instance::appendMove_ecco(const QString& zhStr)
{
    MovSeat movSeat = board_->getMovSeat(zhStr);
    if (!movSeat.first || !board_->isCanMove(movSeat))
        return Q_NULLPTR;

    PMove move = curMove_->addMove(movSeat, zhStr, "", false);
    goNext();

    return move;
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

void Instance::goTo(PMove move)
{
    if (!move || curMove_ == move)
        return;

    backStart();
    for (auto& move : move->getPrevMoveList())
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
    if (!curMove_->preMove())
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

void Instance::backTo(PMove move)
{
    while (curMove_->preMove() && curMove_ != move)
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
        PMove move = isOther ? curMove_->otherMove() : curMove_->nextMove();

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

    PMove curMove { curMove_ };
    backStart();
    if (!board_->changeLayout(ct))
        return false;

    if (rootMove_->nextMove())
        if (!changeMove__(false))
            return false;

    backStart();
    PMove firstMove { rootMove_->nextMove() };
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

bool Instance::isEndMove() const
{
    return !curMove_->nextMove();
}

bool Instance::hasOtherMove() const
{
    return curMove_->otherMove();
}

bool Instance::isOtherMove() const
{
    return curMove_->isOther();
}

QString Instance::getECCORowcols() const
{
    std::function<QString(SeatCoordPair&, ChangeType)>
        getChangeRowcol_ = [](SeatCoordPair& seatCoordPair, ChangeType ct) -> QString {
        seatCoordPair = { Seats::changeSeatCoord(seatCoordPair.first, ct),
            Seats::changeSeatCoord(seatCoordPair.second, ct) };
        return QString("%1%2%3%4")
            .arg(seatCoordPair.first.first)
            .arg(seatCoordPair.first.second)
            .arg(seatCoordPair.second.first)
            .arg(seatCoordPair.second.second);
    };

    QString allRowcols;
    int color = 0;
    PMove move = rootMove_;
    QString rowcol[4][COLORNUM];
    while ((move = move->nextMove())) {
        rowcol[0][color].append(move->rowcols());
        int chIndex = 1;
        SeatCoordPair seatCoordPair = move->seatCoordPair();
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

SeatCoordPair Instance::getCurSeatCoordPair() const
{
    return curMove_->seatCoordPair();
}

QList<SeatCoord> Instance::canMove(SeatCoord seatCoord) const
{
    QList<QList<SeatCoord>> seats = board_->canMove(seatCoord);
    if (!seats.isEmpty())
        return seats.at(0);

    return QList<SeatCoord>();
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

QString Instance::toString(StoreType storeType) const
{
    return InstanceIO::getString(this, storeType);
}

QString Instance::toFullString()
{
    QString qstr {};
    QTextStream stream(&qstr);
    stream << toString(StoreType::PGN_CC) << QString("\n着法描述与棋盘布局：\n");

    std::function<void(const PMove&, bool)>
        __printMoveBoard = [&](const PMove& move, bool isOther) {
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

    PMove curMove { curMove_ };
    backStart();
    if (rootMove_->nextMove())
        __printMoveBoard(rootMove_->nextMove(), false);

    goTo(curMove);
    return qstr;
}

QList<Aspect> Instance::getAspectList()
{
    QList<Aspect> aspectList {};
    PMove curMove = curMove_;
    backStart();
    std::function<void(const PMove&)>
        appendAspect__ = [&](const PMove& move) {
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

void Instance::setMoveNums()
{
    backStart();
    std::function<void(const PMove&)>
        __setNums = [&](const PMove& move) {
            ++movCount_;
            maxCol_ = std::max(maxCol_, move->otherIndex_);
            maxRow_ = std::max(maxRow_, move->nextIndex_);
            move->CC_ColIndex_ = maxCol_; // # 本着在视图中的列数
            if (!move->remark_.isEmpty()) {
                ++remCount_;
                remLenMax_ = std::max(remLenMax_, move->remark_.length());
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

const QString Instance::fen__() const
{
    return info_["FEN"].left(info_["FEN"].indexOf(' '));
}

void Instance::setBoard()
{
    board_->setFEN(fen__());
}

SeatSide Instance::getHomeSide(PieceColor color) const
{
    return board_->getHomeSide(color);
}

QString Instance::getPieceChars() const
{
    return board_->getPieceChars();
}
