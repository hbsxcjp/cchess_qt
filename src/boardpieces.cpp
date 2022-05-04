#include "boardpieces.h"
#include "piece.h"
#include "piecebase.h"
#include "seat.h"

#include <QMap>

BoardPieces::BoardPieces()
    : pieces(Piece::creatPieces())
{
}

BoardPieces::~BoardPieces()
{
    for (auto& colorPieces : pieces)
        for (auto& kindPieces : colorPieces)
            for (auto& piece : kindPieces)
                delete piece;
}

Piece* BoardPieces::getNonLivePiece(PieceColor color, PieceKind kind) const
{
    for (auto& piece : getPieces(color, kind))
        if (!piece->isLive())
            return piece;

    Q_ASSERT("getNotLivePiece ? ");
    return {};
}

Piece* BoardPieces::getOtherPiece(Piece* piece) const
{
    if (!piece)
        return piece;

    PieceColor color = piece->color();
    PieceKind kind = piece->kind();
    int index = getPieces(color, kind).indexOf(piece);
    return getPieces(PieceBase::getOtherColor(color), kind).at(index);
}

QList<Piece*> BoardPieces::getAllPiece() const
{
    QList<Piece*> pieces;
    for (PieceColor color : PieceBase::ALLCOLORS)
        pieces.append(getPieces(color));

    return pieces;
}

Seat* BoardPieces::getKingSeat(PieceColor color) const
{
    return getPieces(color, PieceKind::KING).at(0)->seat();
}

QList<Seat*> BoardPieces::getLiveSeats() const
{
    return getLiveSeats(getAllPiece());
}

QList<Seat*> BoardPieces::getLiveSeats(PieceColor color) const
{
    return getLiveSeats(getPieces(color));
}

QList<Seat*> BoardPieces::getLiveSeats(PieceColor color, PieceKind kind) const
{
    return getLiveSeats(getPieces(color, kind));
}

QList<Seat*> BoardPieces::getLiveSeats(PieceColor color, QChar name) const
{
    return getLiveSeats(color, PieceBase::getKind(color, name));
}

QList<Seat*> BoardPieces::getLiveSeats(PieceColor color, QChar name, int col) const
{
    QList<Seat*> seats { getLiveSeats(color, name) };
    QMutableListIterator<Seat*> seatIter(seats);
    while (seatIter.hasNext())
        if (seatIter.next()->col() != col)
            seatIter.remove();

    return seats;
}

QList<Seat*> BoardPieces::getLiveSeats_SortPawn(PieceColor color, bool isBottom) const
{
    // 最多5个兵, 按列、行建立映射，按列、行排序
    QMap<int, QMap<int, Seat*>> colRowSeats;
    for (auto& seat : getLiveSeats(color, PieceKind::PAWN)) {
        // 根据isBottom值排序
        int col = seat->col(), row = seat->row();
        colRowSeats[isBottom ? -col : col][isBottom ? -row : row] = seat;
    }

    QList<Seat*> seats;
    for (auto& colSeats : colRowSeats.values()) {
        // 只选取2个及以上的QMap
        if (colSeats.size() > 1)
            seats.append(colSeats.values());
    }

    return seats;
}

QString BoardPieces::toString() const
{
    QString string;
    for (auto& piece : getAllPiece())
        string.append(piece->toString());

    return string;
}

QList<Piece*> BoardPieces::getPieces(PieceColor color) const
{
    QList<Piece*> pieces;
    for (PieceKind kind : PieceBase::ALLKINDS)
        pieces.append(getPieces(color, kind));

    return pieces;
}

QList<Piece*> BoardPieces::getPieces(PieceColor color, PieceKind kind) const
{
    return pieces.at(int(color)).at(int(kind));
}

QList<Seat*> BoardPieces::getLiveSeats(const QList<Piece*>& pieces)
{
    QList<Seat*> seats;
    for (auto& piece : pieces)
        if (piece->isLive())
            seats.append(piece->seat());

    return seats;
}
