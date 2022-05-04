#include "seat.h"
#include "boardseats.h"
#include "piece.h"
#include "seatbase.h"

Seat::Seat(const Coord& coord)
    : piece_(Q_NULLPTR)
    , coord_(coord)
{
}

QList<Seat*> Seat::creatSeats()
{
    QList<Seat*> seats;
    for (const Coord& coord : SeatBase::allCoord())
        seats.append(new Seat(coord));

    return seats;
}

bool Seat::isBottom() const
{
    return SeatBase::isBottom(coord_);
}

void Seat::setPiece(Piece* piece)
{
    if (hasPiece())
        piece_->setSeat(Q_NULLPTR);

    if (piece)
        piece->setSeat(this);

    piece_ = piece;
}

void Seat::moveTo(Seat* toSeat, Piece* fillPiece)
{
    Piece* piece = piece_;
    setPiece(fillPiece); // 首先清空this与piece的联系
    toSeat->setPiece(piece); // 清空toSeat与toPiece的联系
}

QList<QList<Coord>> Seat::canMove(const BoardSeats* boardSeats, SeatSide homeSide) const
{
    return SeatBase::canMove(piece_, coord_, boardSeats, homeSide);
}

QString Seat::toString() const
{
    return QString("<%1,%2 %3>").arg(row()).arg(col()).arg(hasPiece() ? piece_->name() : '*');
}
