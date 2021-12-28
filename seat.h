#ifndef SEAT_H
#define SEAT_H
// 棋盘位置类

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

    PPiece piece_;
};

// 一副棋盘位置类
class Seats {
public:
    Seats();
    ~Seats();

    void clear();

    PSeat getSeat(int row, int col) const;
    PSeat getSeat(SeatCoord seatCoord) const;

    PSeat getChangeSeat(PSeat& seat, ChangeType ct) const;
    void changeLayout(const Pieces* pieces, ChangeType ct);

    QString getFEN() const;
    bool setFEN(const Pieces* pieces, const QString& fen);

    QString toString() const;

    static QString pieCharsToFEN(const QString& pieChars);
    static QString FENToPieChars(const QString& fen);

    static int rowcol(int row, int col);
    static int rowcols(int frowcol, int trowcol);
    static QPair<SeatCoord, SeatCoord> seatCoordPair(int rowcols);

    static bool less(PSeat first, PSeat last);
    static bool isBottom(PSeat seat);

    // 棋子可置入位置坐标
    static QList<SeatCoord> allSeatCoord();
    static QList<SeatCoord> kingPutTo(Side homeSide);
    static QList<SeatCoord> advisorPutTo(Side homeSide);
    static QList<SeatCoord> bishopPutTo(Side homeSide);
    static QList<SeatCoord> pawnPutTo(Side homeSide);

    // 在某位置处棋子可移动所至的位置坐标
    static QList<SeatCoord> kingMoveTo(PSeat seat);
    static QList<SeatCoord> advisorMoveTo(PSeat seat, Side homeSide);
    static QList<SeatCoord> bishopMoveTo(PSeat seat);
    static QList<SeatCoord> knightMoveTo(PSeat seat);
    static QList<SeatCoord> rookCannonMoveTo(PSeat seat);
    static QList<SeatCoord> pawnMoveTo(PSeat seat, Side homeSide);

    QList<SeatCoord> bishopRuleFilter(PSeat seat, QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> knightRuleFilter(PSeat seat, QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> rookRuleFilter(PSeat seat, QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> cannonRuleFilter(PSeat seat, QList<SeatCoord>& seatCoordList) const;

private:
    static QList<SeatCoord>& getValidSeatCoord_(QList<SeatCoord>& seatCoordList,
        bool (*isValidFunc)(SeatCoord));
    static bool isValidSeatCoord_(SeatCoord seatCoord);
    static bool isValidKingAdvSeatCoord_(SeatCoord seatCoord);
    static bool isValidBishopSeatCoord_(SeatCoord seatCoord);

    static int symmetryRow_(int row);
    static int symmetryCol_(int col);

    static bool isValidRow_(int row);
    static bool isValidCol_(int col);

    static bool isValidKingAdvRow_(int row);
    static bool isValidKingAdvCol_(int col);

    static bool isValidBishopRow_(int row);

    PSeat seats_[SEATROW][SEATCOL] {};
};

QString printSeatCoord(const SeatCoord& seatCoord);
QString printSeatCoordList(const QList<SeatCoord>& seatCoordList);

QString printSeatList(const QList<PSeat>& seatList);

Q_DECLARE_METATYPE(Side)

#endif // SEAT_H
