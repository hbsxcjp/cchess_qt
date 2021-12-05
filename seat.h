#ifndef SEAT_H
#define SEAT_H

#include <QList>
#include <QMetaType>
#include <QPair>
#include <QString>

#define SEATROW 10
#define SEATCOL 9
#define SEATNUM (SEATROW * SEATCOL)

class Seat;
class Seats;
using PSeat = Seat*;
using SeatCoord = QPair<int, int>;

class Piece;
using PPiece = Piece*;

class Pieces;

enum class Side {
    HERE,
    THERE
};

enum class ChangeType {
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
    SeatCoord seatCoord() const { return { row_, col_ }; }

    PPiece getPiece() const { return piece_; }
    PSeat setPiece(PPiece piece);

    PPiece moveTo(PSeat toSeat, PPiece fillPiece = nullptr);

    QString toString() const;

private:
    Seat(int row, int col);

    const int row_;
    const int col_;

    PPiece piece_ {};
};

// 一副棋盘位置类
class Seats {
public:
    Seats();
    ~Seats();

    void clear();

    PSeat getSeat(int row, int col) const { return seats_[row][col]; };
    PSeat getSeat(int rowcol) const { return getSeat(rowcol / 10, rowcol % 10); };
    PSeat getSeat(SeatCoord seatCoord) const { return getSeat(seatCoord.first, seatCoord.second); };

    QList<PSeat> getSeatList(const QList<SeatCoord>& seatCoords) const;

    PSeat getChangeSeat(PSeat& seat, ChangeType ct) const;
    void changeLayout(const Pieces* pieces, ChangeType ct);

    QString getPieChars() const;
    bool setPieChars(const Pieces* pieces, const QString& pieChars);

    QString getFEN() const;
    bool setFEN(const Pieces* pieces, const QString& fen);

    QString toString() const;

    static QString pieCharsToFEN(const QString& pieChars);
    static QString FENToPieChars(const QString& fen);

    static int rowcol(int row, int col) { return row * 10 + col; }
    static SeatCoord seatCoord(int rowcol) { return { rowcol / 10, rowcol % 10 }; }

    static int symmetryRow(int row) { return SEATROW - 1 - row; }
    static int symmetryCol(int col) { return SEATCOL - 1 - col; }

    static bool isValidRow(int row) { return row >= 0 && row < SEATROW; }
    static bool isValidCol(int col) { return col >= 0 && col < SEATCOL; }

private:
    PSeat seats_[SEATROW][SEATCOL] {};
};

Q_DECLARE_METATYPE(Side)

#endif // SEAT_H
