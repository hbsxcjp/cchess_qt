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

enum class Side {
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
    PSeat getSeat(int rowcol) const { return getSeat(rowcol / 10, rowcol % 10); };
    PSeat getSeat(QPair<int, int> rcPair) const { return getSeat(rcPair.first, rcPair.second); };

    PSeat getChangeSeat(PSeat seat, ChangeType ct) const;

    // 棋子可至全部位置
    QList<PSeat> allSeats() const;

    // 棋子可置入位置
    QList<PSeat> put(PPiece piece, Side homeSide) const;

    // 棋子从某位置可移至位置
    QList<PSeat> move(PSeat seat, Side homeSide) const;

    static int rowcol(int row, int col) { return row * 10 + col; }
    static QPair<int, int> rcPair(int rowcol) { return { rowcol / 10, rowcol % 10 }; }

    static int symmetryRow(int row) { return SEATROW - 1 - row; }
    static int symmetryCol(int col) { return SEATCOL - 1 - col; }

    static bool isValidRow(int row) { return row >= 0 && row < SEATROW; }
    static bool isValidCol(int col) { return col >= 0 && col < SEATCOL; }

private:
    PSeat seats_[SEATROW][SEATCOL] {};
};

QString printSeatList(const QList<PSeat>& seatList);

Q_DECLARE_METATYPE(Side)

#endif // SEAT_H
