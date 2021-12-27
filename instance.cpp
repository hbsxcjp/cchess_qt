#include "instance.h"
#include "aspect.h"
#include "board.h"
#include "instanceio.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"

Instance::Instance()
    : board_(new Board())
    , rootMove_(new Move())
    , curMove_(rootMove_)
    , info_({ { "TITLE", "" }, { "EVENT", "" }, { "DATE", "" },
          { "SITE", "" }, { "BLACK", "" }, { "RED", "" },
          { "OPENING", "" }, { "WRITER", "" }, { "AUTHOR", "" },
          { "TYPE", "" }, { "RESULT", "" }, { "VERSION", "" },
          { "SOURCE", "" }, { "FEN", Pieces::FENStr }, { "ICCSSTR", "" },
          { "ECCOSN", "" }, { "ECCONAME", "" }, { "MOVESTR", "" } })
{
}

Instance::~Instance()
{
    Move::deleteMove(rootMove_); // 驱动函数
    delete board_;
}

PMove Instance::appendMove(const MovSeat& movSeat, const QString& remark, bool isOther)
{
    if (isOther)
        curMove_->undo();
    Q_ASSERT(movSeat.first->getPiece());
    QString zhStr { board_->getZhStr(movSeat) };

#ifdef DEBUG
    /*// 观察棋盘局面与着法
    Tools::writeTxtFile("test.txt",
        (movSeat.first->toString() + movSeat.second->toString()
            + zhStr + (isOther ? " isOther.\n" : "\n")),
        QIODevice::Append);
    //*/
#endif

    // 疑难文件通不过下面的检验，需注释此行
    Q_ASSERT(board_->isCanMove(movSeat));
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

PMove Instance::appendMove(int rowcols, const QString& remark, bool isOther)
{
    return appendMove(board_->getMovSeat(rowcols), remark, isOther);
}

PMove Instance::appendMove(SeatCoordPair seatCoordlPair, const QString& remark, bool isOther)
{
    return appendMove(board_->getMovSeat(seatCoordlPair), remark, isOther);
}

PMove Instance::appendMove(QList<QChar> iccs, const QString& remark, bool isOther)
{
    return appendMove(SeatCoordPair { { Pieces::getRowFrom(iccs[1]),
                                          Pieces::getColFrom(iccs[0]) },
                          { Pieces::getRowFrom(iccs[3]),
                              Pieces::getColFrom(iccs[2]) } },
        remark, isOther);
}

PMove Instance::appendMove(QString zhStr, const QString& remark, bool isOther)
{
    if (isOther)
        curMove_->undo();
    MovSeat movseat = board_->getMovSeat(zhStr);
    if (isOther)
        curMove_->done();

    return appendMove(movseat, remark, isOther);
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
    Q_ASSERT(move);
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

void Instance::goInc(int inc)
{
    int incCount { abs(inc) };
    // std::function<void(Instance*)> fbward = inc > 0 ? &Instance::go : &Instance::back;
    auto fbward = std::mem_fn(inc > 0 ? &Instance::goNext : &Instance::backOne);
    while (incCount-- && fbward(this))
        ;
}

void Instance::changeLayout(ChangeType ct)
{
    std::function<void(bool)>
        changeMove__ = [&](bool isOther) {
            PMove move = isOther ? curMove_->otherMove() : curMove_->nextMove();

            if (isOther)
                curMove_->undo();
            move->changeLayout(board_, ct);
            if (isOther)
                curMove_->done();

            go(isOther);
            if (move->nextMove())
                changeMove__(false);

            if (move->otherMove())
                changeMove__(true);
            back(isOther);
        };

    PMove curMove { curMove_ };
    backStart();
    board_->changeLayout(ct);
    if (rootMove_->nextMove())
        changeMove__(false);

    backStart();
    PMove firstMove { rootMove_->nextMove() ? rootMove_->nextMove() : nullptr };
    setFEN(board_->getFEN(), firstMove ? firstMove->color() : Color::RED);

    goTo(curMove);
}

QString Instance::getZhChars() const
{
    return board_->getZhChars();
}

SeatCoordPair Instance::getCurSeatCoordPair() const
{
    auto movSeat = curMove_->movSeat();
    if (movSeat.first && movSeat.second)
        return { movSeat.first->seatCoord(), movSeat.second->seatCoord() };

    return { { -1, -1 }, { -1, -1 } };
}

const QString Instance::toString()
{
    return InstanceIO::pgnString(this, PGN::CC);
}

const QString Instance::toFullString()
{
    QString qstr {};
    QTextStream stream(&qstr);
    stream << toString() << QString("\n着法描述与棋盘布局：\n");

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
            Color color = move->color();
            aspectList.append(Aspect(board_->getFEN(), color, move->rowcols()));

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
            maxCol_ = std::max(maxCol_, move->otherNo_);
            maxRow_ = std::max(maxRow_, move->nextNo_);
            move->CC_ColNo_ = maxCol_; // # 本着在视图中的列数
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

void Instance::setFEN(const QString& fen, Color color)
{
    info_["FEN"] = (fen + " "
        + (color == Color::RED ? "r" : "b") + " - - 0 1");
}

const QString Instance::fen__() const
{
    return info_["FEN"].left(info_["FEN"].indexOf(' '));
}

void Instance::setBoard()
{
    board_->setFEN(fen__());
}

const QString Instance::moveInfo() const
{
    return QString::asprintf(
        "【着法深度：%d, 视图宽度：%d, 着法数量：%d, 注解数量：%d, 注解最长：%d】\n",
        maxRow_, maxCol_, movCount_, remCount_, remLenMax_);
}
