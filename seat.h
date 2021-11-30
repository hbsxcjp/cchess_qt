#ifndef SEAT_H
#define SEAT_H

#include "piece.h"
#include <QList>
#include <QMetaType>
#include <QPair>
#include <QString>

class Seat;
using PSeat = Seat*;

class Seats;

enum Side {
    HERE,
    THERE
};

enum ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY,
    NOCHANGE
};

// 位置类
class Seat {
    friend Seats;

public:
    int row() const { return row_; }

    int col() const { return col_; }

    int rowcol() const { return row_ * 10 + col_; }

    int symmetryRow() const { return SEATROW - 1 - row_; }

    int symmetryCol() const { return SEATCOL - 1 - col_; }

    PPiece getPiece() const { return piece_; }

    QString toString() const { return QString("<%1,%2>").arg(row_).arg(col_); }

private:
    Seat(int row, int col);

    int row_;
    int col_;

    PPiece piece_ {};
};

// 一副棋盘位置类
class Seats {
public:
    Seats();
    ~Seats();

    PSeat getSeat(int row, int col) const { return seats_[row][col]; };

    PSeat getChangeSeat(PSeat seat, ChangeType ct) const;

    // 棋子可至全部位置
    QList<PSeat> allSeats() const;

    // 棋子可置入位置
    QList<PSeat> put(PPiece piece, Side homeSide) const;

    // 棋子从某位置可移至位置
    QList<PSeat> move(PSeat seat, Side homeSide) const;

private:
    PSeat seats_[SEATROW][SEATCOL] {};
};

QString printSeatList(const QList<PSeat>& seatList);

Q_DECLARE_METATYPE(Side)

#endif // SEAT_H
