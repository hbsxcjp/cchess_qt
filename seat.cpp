#include "seat.h"
#include "piece.h"
#include <functional>

Seat::Seat(int row, int col)
    : row_(row)
    , col_(col)
{
}

Seats::Seats()
{
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seats_[row][col] = new Seat(row, col);
}

Seats::~Seats()
{
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            delete seats_[row][col];
}

PSeat Seats::getChangeSeat(PSeat seat, ChangeType ct) const
{
    if (ct == ChangeType::SYMMETRY)
        return getSeat(seat->row(), symmetryCol(seat->col()));
    else if (ct == ChangeType::ROTATE)
        return getSeat(symmetryRow(seat->row()), symmetryCol(seat->col()));
    else
        //(ct == ChangeType::NOCHANGE || ct == ChangeType::EXCHANGE)
        return seat;
}

QList<PSeat> Seats::allSeats() const
{
    QList<PSeat> seats;
    for (int row = 0; row < SEATROW; ++row)
        for (int col = 0; col < SEATCOL; ++col)
            seats.append(seats_[row][col]);

    return seats;
}

QList<PSeat> Seats::put(PPiece piece, Side homeSide) const
{
    QList<PSeat> seatList;
    for (auto& rcPair : piece->putRowCol(homeSide))
        seatList.append(getSeat(rcPair));

    return seatList;
}

QList<PSeat> Seats::move(PSeat seat, Side homeSide) const
{
    QList<PSeat> seatList;
    PPiece piece = seat->getPiece();
    if (!piece)
        return seatList;

    for (auto& rcPair : piece->moveRowCol(seat->row(), seat->col(), homeSide))
        seatList.append(getSeat(rcPair));

    return seatList;
}

QString printSeatList(const QList<PSeat>& seatList)
{
    QString qstr {};
    int count = seatList.count();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            qstr.append(seatList[i]->toString());
            // 每行SEATCOL个数据
            if ((i % SEATCOL == SEATCOL - 1) && i != count - 1)
                qstr.append("\n");
        }
        qstr.append(QString("【%1】").arg(count));
    }

    return qstr;
}
