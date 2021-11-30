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
        return getSeat(seat->row(), seat->symmetryCol());
    else if (ct == ChangeType::ROTATE)
        return getSeat(seat->symmetryRow(), seat->symmetryCol());
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
    switch (piece->kind()) {
    case Kind::KING:
        for (int row = 0; row < 3; ++row)
            for (int col = 3; col < 6; ++col)
                seatList.append(getSeat(row, col));
        break;
    case Kind::ADVISOR:
        for (int row = 0; row < 3; ++row)
            for (int col = 3; col < 6; ++col)
                if (row % 2 != col % 2)
                    seatList.append(getSeat(row, col));
        break;
    case Kind::BISHOP:
        for (int row = 0; row < SEATROW / 2; ++row)
            for (int col = 0; col < SEATCOL; ++col)
                if (row % 2 == 0 && col % 2 == 0 && qAbs(col - row) % 4 != 0)
                    seatList.append(getSeat(row, col));
        break;
    case Kind::KNIGHT:
    case Kind::ROOK:
    case Kind::CANNON:
        return allSeats();
    default:
        // case Kind::PAWN:
        for (int row = 3; row < SEATROW; ++row)
            for (int col = 0; col < SEATCOL; ++col)
                if (row > 4 || col % 2 == 0)
                    seatList.append(getSeat(row, col));
        break;
    }

    if (homeSide == Side::THERE)
        for (auto& seat : seatList)
            seat = getChangeSeat(seat, ChangeType::ROTATE);

    return seatList;
}

QList<PSeat> Seats::move(PSeat seat, Side homeSide) const
{
    std::function<void(QList<PSeat> & seatList, QVector<QPair<bool, QPair<int, int>>>)>
        getSeats_ = [&](QList<PSeat>& seatList, QVector<QPair<bool, QPair<int, int>>> keepSeats) {
            for (int i = 0; i < keepSeats.count(); ++i)
                if (keepSeats[i].first) {
                    auto& rowcolPair = keepSeats[i].second;
                    seatList.append(this->getSeat(rowcolPair.first, rowcolPair.second));
                }
        };

    QList<PSeat> seatList;
    PPiece piece = seat->getPiece();
    if (!piece)
        return seatList;

    int row = seat->row(), col = seat->col();
    switch (piece->kind()) {
    case Kind::KING:
        if (row != 0 || row != 7)
            seatList.append(getSeat(row - 1, col));
        if (row != 2 || row != 9)
            seatList.append(getSeat(row + 1, col));
        if (col != 3)
            seatList.append(getSeat(row, col - 1));
        if (col != 5)
            seatList.append(getSeat(row, col + 1));
        break;
    case Kind::ADVISOR:
        if (row == 4)
            seatList.append({ getSeat(row - 1, col - 1), getSeat(row - 1, col + 1),
                getSeat(row + 1, col - 1), getSeat(row + 1, col + 1) });
        else
            seatList.append(getSeat((row < SEATROW / 2) ? 1 : 8, 4));
        break;
    case Kind::BISHOP:
        if (row == 0 || row == 5)
            seatList.append({ getSeat(row + 2, col - 2), getSeat(row + 2, col + 2) });
        else if (row == 4 || row == 9)
            seatList.append({ getSeat(row - 2, col - 2), getSeat(row - 2, col + 2) });
        else if (col == 0)
            seatList.append({ getSeat(row - 2, col + 2), getSeat(row + 2, col + 2) });
        else if (col == SEATCOL - 1)
            seatList.append({ getSeat(row - 2, col - 2), getSeat(row + 2, col - 2) });
        else
            seatList.append({ getSeat(row - 2, col - 2), getSeat(row - 2, col + 2),
                getSeat(row + 2, col - 2), getSeat(row + 2, col + 2) });
        break;
    case Kind::KNIGHT: {
        QVector<QPair<bool, QPair<int, int>>> keepSeats {
            { true, { row - 2, col - 1 } }, { true, { row - 2, col + 1 } },
            { true, { row - 1, col - 2 } }, { true, { row - 1, col + 2 } },
            { true, { row + 1, col - 2 } }, { true, { row + 1, col + 2 } },
            { true, { row + 2, col - 1 } }, { true, { row + 2, col + 1 } }
        };
        if (row < 2) {
            keepSeats[0].first = false;
            keepSeats[1].first = false;
            if (row == 0) {
                keepSeats[2].first = false;
                keepSeats[3].first = false;
            }
        } else if (row > SEATROW - 3) {
            keepSeats[6].first = false;
            keepSeats[7].first = false;
            if (row == SEATROW - 1) {
                keepSeats[4].first = false;
                keepSeats[5].first = false;
            }
        }
        if (col < 2) {
            keepSeats[2].first = false;
            keepSeats[4].first = false;
            if (col == 0) {
                keepSeats[0].first = false;
                keepSeats[6].first = false;
            }
        } else if (col > SEATCOL - 3) {
            keepSeats[3].first = false;
            keepSeats[5].first = false;
            if (col == SEATCOL - 1) {
                keepSeats[1].first = false;
                keepSeats[7].first = false;
            }
        }
        getSeats_(seatList, keepSeats);
    } break;
    case Kind::ROOK:
    case Kind::CANNON:
        // 先行后列，先小后大。顺序固定，为Board::canMove()分析走棋规则打下基础
        for (int r = row - 1; r >= 0; --r)
            seatList.append(getSeat(r, col));
        for (int r = row + 1; r < SEATROW; ++r)
            seatList.append(getSeat(r, col));
        for (int c = col - 1; c >= 0; --c)
            seatList.append(getSeat(row, c));
        for (int c = col + 1; c < SEATCOL; ++c)
            seatList.append(getSeat(row, c));
        break;
    case Kind::PAWN: {
        QVector<QPair<bool, QPair<int, int>>> keepSeats {
            { true, { row - 1, col } }, { true, { row + 1, col } },
            { true, { row, col - 1 } }, { true, { row, col + 1 } }
        };
        if (homeSide == Side::HERE)
            keepSeats[0].first = false;
        else
            keepSeats[1].first = false;
        if (col == SEATCOL - 1)
            keepSeats[3].first = false;
        else if (col == 0)
            keepSeats[2].first = false;

        // 已过河
        if ((row >= SEATROW / 2) == (homeSide == Side::HERE)) {
            if (row == SEATROW - 1)
                keepSeats[1].first = false;
            else if (row == 0)
                keepSeats[0].first = false;
        } else {
            keepSeats[2].first = false;
            keepSeats[3].first = false;
            keepSeats[homeSide == Side::HERE ? 0 : 1].first = false;
        }
        getSeats_(seatList, keepSeats);
    } break;
    default:
        break;
    }
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
