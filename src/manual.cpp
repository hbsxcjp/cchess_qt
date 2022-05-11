#include "manual.h"
#include "aspect.h"
#include "board.h"
#include "boardpieces.h"
#include "boardseats.h"
#include "manualIO.h"
#include "manualmove.h"
#include "manualmoveiterator.h"
#include "move.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"
#include "seatbase.h"
#include "tools.h"

Manual::Manual()
    : board_(new Board)
    , manualMove(new ManualMove)
    , info_(InfoMap())
    , status_(ManualStatus::MOVEDEMO)
{
}

Manual::~Manual()
{
    delete manualMove;
    delete board_;
}

void Manual::reset()
{
    delete manualMove;
    manualMove = new ManualMove;
    board_->init();
}

QList<Piece*> Manual::getAllPiece() const
{
    return board_->getAllPiece();
}

QList<Seat*> Manual::getLiveSeats() const
{
    return board_->getLiveSeats();
}

Move* Manual::appendMove(const CoordPair& coordPair, const QString& remark, bool isOther)
{
    return manualMove->appendMove(board_, board_->getSeatPair(coordPair), remark, isOther);
}

Move* Manual::appendMove(const QString& iccsOrZhStr, const QString& remark, bool isOther, bool isPGN_ZH)
{
    if (!isPGN_ZH) {
        CoordPair coordPair { { PieceBase::getRowFrom(iccsOrZhStr[1]), PieceBase::getColFrom(iccsOrZhStr[0]) },
            { PieceBase::getRowFrom(iccsOrZhStr[3]), PieceBase::getColFrom(iccsOrZhStr[2]) } };
        return appendMove(coordPair, remark, isOther);
    }

    Move* curMove { manualMove->move() };
    if (isOther)
        //        manualMove->backToPre();
        curMove->undo();

    SeatPair seatPair = board_->getSeatPair(iccsOrZhStr);
    if (isOther)
        //        manualMove->goToOther(curMove);
        curMove->done();

    return manualMove->appendMove(board_, seatPair, remark, isOther);
}

Move* Manual::appendMove(const QString& rowcols, const QString& remark, bool isOther)
{
    return appendMove(SeatBase::coordPair(rowcols), remark, isOther);
}

Move* Manual::appendMove(const QString& zhStr)
{
    return appendMove(zhStr, "", false, true);
}

bool Manual::changeLayout(ChangeType ct)
{
    std::function<bool(bool)>
        changeMove__ = [&](bool isOther) -> bool {
        Move* curMove { manualMove->move() };
        Move* move = isOther ? curMove->otherMove() : curMove->nextMove();

        if (isOther)
            curMove->undo();
        if (!move->changeLayout(board_, ct))
            return false;
        if (isOther)
            curMove->done();

        manualMove->goIs(isOther);
        if (move->hasNext())
            if (!changeMove__(false))
                return false;

        if (move->hasOther())
            if (!changeMove__(true))
                return false;

        manualMove->backIs(isOther);
        return true;
    };

    Move* curMove { manualMove->move() };
    manualMove->backStart();
    if (!board_->changeLayout(ct))
        return false;

    //    ManualMoveFirstNextIterator firstNextIter(manualMove);
    //    while (firstNextIter.hasNext()) {
    //        Move* move = firstNextIter.next();
    //        Q_ASSERT(move->changeLayout(board_, ct));
    //    }
    if (!manualMove->isEmpty())
        if (!changeMove__(false))
            return false;
    manualMove->backStart();

    setFEN(board_->getFEN(), manualMove->firstColor());

    manualMove->goTo(curMove);
    return true;
}

QString Manual::getInfoValue(InfoIndex nameIndex)
{
    return info_[ManualIO::getInfoName(nameIndex)];
}

void Manual::setInfoValue(InfoIndex nameIndex, const QString& value)
{
    info_[ManualIO::getInfoName(nameIndex)] = value;
}

Move* Manual::getRootMove() const
{
    return manualMove->rootMove();
}

Move* Manual::getCurMove() const
{
    return manualMove->move();
}

QString Manual::getECCORowcols() const
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
    QString rowcol[4][PieceBase::ALLCOLORS.size()];

    //    Move* move = manualMove->rootMove();
    //    while ((move = move->nextMove())) {
    //        rowcol[0][color].append(move->rowcols());
    //        int chIndex = 1;
    //        CoordPair coordPair = move->coordPair();
    //        for (ChangeType ct : { ChangeType::SYMMETRY_H, ChangeType::ROTATE, ChangeType::SYMMETRY_H })
    //            rowcol[chIndex++][color].append(getChangeRowcol_(coordPair, ct));

    //        color = (color + 1) % 2;
    //    }
    ManualMoveOnlyNextIterator onlyNextIter(manualMove);
    while (onlyNextIter.hasDoneNext()) {
        Move* move = onlyNextIter.doneNext();

        rowcol[0][color].append(move->rowcols());
        int chIndex = 1;
        CoordPair coordPair = move->coordPair();
        for (ChangeType ct : { ChangeType::SYMMETRY_H, ChangeType::ROTATE, ChangeType::SYMMETRY_H })
            rowcol[chIndex++][color].append(getChangeRowcol_(coordPair, ct));

        color = (color + 1) % 2;
    }

    for (int chIndex = 0; chIndex < 4; ++chIndex)
        allRowcols.append(QString("~%1-%2").arg(rowcol[chIndex][0]).arg(rowcol[chIndex][1]));

    return allRowcols;
}

void Manual::setEcco(const QStringList& eccoRec)
{
    info_[ManualIO::getInfoName(InfoIndex::ECCOSN)] = eccoRec.at(0);
    info_[ManualIO::getInfoName(InfoIndex::ECCONAME)] = eccoRec.at(1);
}

SeatPair Manual::getCurSeatPair() const
{
    return getCurMove()->seatPair();
}

CoordPair Manual::getCurCoordPair() const
{
    return getCurMove()->coordPair();
}

QList<Coord> Manual::canPut(Piece* piece) const
{
    return SeatBase::canPut(piece->kind(), getHomeSide(piece->color()));
}

QList<Coord> Manual::canMove(const Coord& coord) const
{
    return board_->canMove(coord).value(0);
}

void Manual::setMoveNums()
{
    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
    ManualMoveFirstNextIterator firstNextIter(manualMove);
    while (firstNextIter.hasDoneNext()) {
        Move* move = firstNextIter.doneNext();

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

void Manual::setFEN(const QString& fen, PieceColor color)
{
    info_["FEN"] = QString("%1 %2 - - 0 1").arg(fen).arg((color == PieceColor::RED ? "r" : "b"));
}

void Manual::setBoard()
{
    const QString& fen = info_["FEN"];
    board_->setFEN(fen.left(fen.indexOf(' ')));
}

SeatSide Manual::getHomeSide(PieceColor color) const
{
    return board_->getHomeSide(color);
}

QString Manual::getPieceChars() const
{
    return board_->getPieceChars();
}

QString Manual::moveInfo() const
{
    return QString("【着法深度：%1, 视图宽度：%2, 着法数量：%3, 注解数量：%4, 注解最长：%5】\n")
        .arg(maxRow_)
        .arg(maxCol_)
        .arg(movCount_)
        .arg(remCount_)
        .arg(remLenMax_);
}

QString Manual::boardString(bool full) const
{
    return getCurMove()->toString() + board_->toString(full);
}

QString Manual::toMoveString(StoreType storeType) const
{
    return ManualIO::getMoveString(this, storeType);
}

QString Manual::toString(StoreType storeType) const
{
    return ManualIO::getString(this, storeType);
}

QString Manual::toFullString()
{
    QString string {};
    QTextStream stream(&string);
    stream << toString(StoreType::PGN_CC) << QString("\n着法描述与棋盘布局：\n");

    ManualMoveFirstNextIterator firstNextIter(manualMove);
    while (firstNextIter.hasDoneNext()) {
        Move* move = firstNextIter.doneNext();
        move->undo();

        stream << move->toString();
        QStringList boardString0 = board_->toString().split('\n');

        move->done();
        QStringList boardString1 = board_->toString().split('\n');
        for (int i = 0; i < boardString0.count() - 1; ++i) // 最后有个回车符是空行
            stream << boardString0.at(i) + "  " + boardString1.at(i) + '\n';
        stream << move->toString() << "\n\n";
    }

    return string;
}

QList<Aspect> Manual::getAspectList()
{
    QList<Aspect> aspectList {};
    ManualMoveFirstNextIterator firstNextIter(manualMove);
    while (firstNextIter.hasNext()) {
        Move* move = firstNextIter.next();
        //        move->undo();
        aspectList.append(Aspect(board_->getFEN(), move->color(), move->rowcols()));
        //        move->done();
    }

    return aspectList;
}