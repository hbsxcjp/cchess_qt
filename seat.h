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
enum class Color;

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
    int rowcol() const;

    PPiece getPiece() const { return piece_; }
    void setPiece(PPiece piece);

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
    PSeat getSeat(SeatCoord seatCoord) const { return getSeat(seatCoord.first, seatCoord.second); };

    PSeat getChangeSeat(PSeat& seat, ChangeType ct) const;
    void changeLayout(const Pieces* pieces, ChangeType ct);

    QString getFEN() const;
    bool setFEN(const Pieces* pieces, const QString& fen);

    QString toString() const;

    static QString pieCharsToFEN(const QString& pieChars);
    static QString FENToPieChars(const QString& fen);

    static int rowcol(int row, int col) { return row * 10 + col; }
    static int rowcols(int frowcol, int trowcol) { return frowcol * 100 + trowcol; }
    static QPair<SeatCoord, SeatCoord> seatCoordPair(int rowcols)
    {
        return { { rowcols / 1000, (rowcols / 100) % 10 }, { (rowcols % 100) / 10, rowcols % 10 } };
    }

    static bool isLess(PSeat first, PSeat last)
    {
        return (first->row() < last->row()
            || (first->row() == last->row() && first->col() < last->col()));
    }
    static bool isBottom(PSeat seat) { return seat->row() < SEATROW / 2; }

    static QList<SeatCoord> allSeatCoord();
    static QList<SeatCoord> kingSeatCoord(Side homeSide);
    static QList<SeatCoord> advisorSeatCoord(Side homeSide);
    static QList<SeatCoord> bishopSeatCoord(Side homeSide);
    static QList<SeatCoord> pawnSeatCoord(Side homeSide);

    static QList<SeatCoord> kingMoveSeatCoord(PSeat seat);
    static QList<SeatCoord> advisorMoveSeatCoord(PSeat seat, Side homeSide);
    static QList<SeatCoord> bishopMoveSeatCoord(PSeat seat);
    static QList<SeatCoord> knightMoveSeatCoord(PSeat seat);
    static QList<SeatCoord> rookCannonMoveSeatCoord(PSeat seat);
    static QList<SeatCoord> pawnMoveSeatCoord(PSeat seat, Side homeSide);

    QList<SeatCoord> bishopFilterRuleSeatCoord(PSeat seat, QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> knightFilterRuleSeatCoord(PSeat seat, QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> rookFilterRuleSeatCoord(PSeat seat, QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> cannonFilterRuleSeatCoord(PSeat seat, QList<SeatCoord>& seatCoordList) const;

    static QList<SeatCoord>& getValidSeatCoord(QList<SeatCoord>& seatCoordList,
        bool (*isValidFunc)(SeatCoord));
    static bool isValidSeatCoord(SeatCoord seatCoord);
    static bool isValidKingAdvSeatCoord(SeatCoord seatCoord);
    static bool isValidBishopSeatCoord(SeatCoord seatCoord);

private:
    static int symmetryRow_(int row) { return SEATROW - 1 - row; }
    static int symmetryCol_(int col) { return SEATCOL - 1 - col; }

    static bool isValidRow_(int row) { return row >= 0 && row < SEATROW; }
    static bool isValidCol_(int col) { return col >= 0 && col < SEATCOL; }

    static bool isValidKingAdvRow_(int row) { return (row >= 0 && row < 3) || (row >= 7 && row < SEATROW); }
    static bool isValidKingAdvCol_(int col) { return col >= 3 && col < 6; }

    static bool isValidBishopRow_(int row) { return QList<int>({ 0, 2, 4, 5, 7, 9 }).contains(row); }

    PSeat seats_[SEATROW][SEATCOL] {};
};

QString printSeatCoord(const SeatCoord& seatCoord);
QString printSeatCoordList(const QList<SeatCoord>& seatCoordList);

QString printSeatList(const QList<PSeat>& seatList);

Q_DECLARE_METATYPE(Side)

#endif // SEAT_H
