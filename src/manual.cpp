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
    , manualMove_(new ManualMove)
    , info_(InfoMap())
    , status_(ManualStatus::MOVEDEMO)
{
}

Manual::~Manual()
{
    delete manualMove_;
    delete board_;
}

void Manual::reset()
{
    delete manualMove_;
    manualMove_ = new ManualMove;
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

Move* Manual::goAppendMove(const CoordPair& coordPair, const QString& remark, bool isOther)
{
    return manualMove_->goAppendMove(board_, board_->getSeatPair(coordPair), remark, isOther);
}

Move* Manual::goAppendMove(const QString& iccsOrZhStr, const QString& remark, bool isPGN_ZH, bool isOther)
{
    if (!isPGN_ZH) {
        CoordPair coordPair { { PieceBase::getRowFrom(iccsOrZhStr[1]), PieceBase::getColFrom(iccsOrZhStr[0]) },
            { PieceBase::getRowFrom(iccsOrZhStr[3]), PieceBase::getColFrom(iccsOrZhStr[2]) } };
        return goAppendMove(coordPair, remark, isOther);
    }

    Move* curMove { manualMove_->move() };
    if (isOther)
        curMove->undo();

    SeatPair seatPair = board_->getSeatPair(iccsOrZhStr);
    if (isOther)
        curMove->done();

    return manualMove_->goAppendMove(board_, seatPair, remark, isOther);
}

Move* Manual::goAppendMove(const QString& rowcols, const QString& remark, bool isOther)
{
    return goAppendMove(SeatBase::coordPair(rowcols), remark, isOther);
}

Move* Manual::goAppendMove(const QString& zhStr)
{
    return goAppendMove(zhStr, "", true, false);
}

bool Manual::changeLayout(ChangeType ct)
{
    Move* curMove { manualMove_->move() };
    manualMove_->backStart();
    if (!board_->changeLayout(ct))
        return false;

    ManualMoveFirstNextIterator firstNextIter(manualMove_);
    while (firstNextIter.hasNext()) {
        Move* move = firstNextIter.next();
        //        Q_ASSERT(move->changeLayout(board_, ct));
        move->changeLayout(board_, ct);
    }

    setFEN(board_->getFEN(), manualMove_->firstColor());

    manualMove_->goTo(curMove);
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

    ManualMoveOnlyNextIterator onlyNextIter(manualMove_);
    while (onlyNextIter.hasNext()) {
        Move* move = onlyNextIter.next();

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

QList<Coord> Manual::canPut(Piece* piece) const
{
    return SeatBase::canPut(piece->kind(), getHomeSide(piece->color()));
}

QList<Coord> Manual::canMove(const Coord& coord) const
{
    return board_->canMove(coord).value(0);
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

QString Manual::boardString(bool full) const
{
    return manualMove()->move()->toString() + board_->toString(full);
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

    ManualMoveFirstNextIterator firstNextIter(manualMove_);
    while (firstNextIter.hasNext()) {
        Move* move = firstNextIter.next();
        stream << move->toString();
        QStringList boardString0 = board_->toString().split('\n');

        move->done();
        QStringList boardString1 = board_->toString().split('\n');
        for (int i = 0; i < boardString0.count() - 1; ++i) // 最后有个回车符是空行
            stream << boardString0.at(i) + "  " + boardString1.at(i) + '\n';
        stream << move->toString() << "\n\n";
        move->undo();
    }

    return string;
}

QList<Aspect> Manual::getAspectList()
{
    QList<Aspect> aspectList {};
    ManualMoveFirstNextIterator firstNextIter(manualMove_);
    while (firstNextIter.hasNext()) {
        Move* move = firstNextIter.next();
        aspectList.append(Aspect(board_->getFEN(), move->color(), move->rowcols()));
    }

    return aspectList;
}
