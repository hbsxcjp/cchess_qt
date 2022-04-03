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
enum class PieceColor;

enum class SeatSide {
    BOTTOM,
    TOP
};

enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY_H,
    SYMMETRY_V,
    NOCHANGE
};

// 位置类
class Seat {
    friend Seats;

public:
    int row() const { return row_; }
    int col() const { return col_; }
    SeatCoord seatCoord() const { return { row_, col_ }; }
    QString rowcol() const;

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

    static SeatSide showHomeSide(SeatSide homeSide);
    static SeatCoord changeSeatCoord(SeatCoord seatCoord, ChangeType ct);
    PSeat changeSeat(PSeat& seat, ChangeType ct) const;
    void changeLayout(const Pieces* pieces, ChangeType ct);

    QString getPieceChars() const;
    bool setPieceChars(const Pieces* pieces, const QString& pieceChars);
    QString getFEN() const;
    bool setFEN(const Pieces* pieces, const QString& fen);

    QString toString() const;

    static QString pieCharsToFEN(const QString& pieChars);
    static QString FENToPieChars(const QString& fen);

    static QString rowcol(int row, int col);
    static QString rowcols(const QString& frowcol, const QString& trowcol);
    static QPair<SeatCoord, SeatCoord> seatCoordPair(const QString& rowcols);
    static SeatCoord getSeatCoord(int index);
    static int getBoardIndex(const SeatCoord& seatCoord);

    static bool less(PSeat first, PSeat last);
    static bool isBottom(PSeat seat);

    static QList<SeatCoord> getSeatCoordList(const QList<PSeat>& seatList);

    // 棋子可置入位置坐标
    static QList<SeatCoord> allSeatCoord();
    static QList<SeatCoord> kingPutTo(SeatSide homeSide);
    static QList<SeatCoord> advisorPutTo(SeatSide homeSide);
    static QList<SeatCoord> bishopPutTo(SeatSide homeSide);
    static QList<SeatCoord> pawnPutTo(SeatSide homeSide);

    // 在某位置处棋子可移动所至的位置坐标
    static QList<SeatCoord> kingMoveTo(PSeat seat);
    static QList<SeatCoord> advisorMoveTo(PSeat seat, SeatSide homeSide);
    static QList<SeatCoord> bishopMoveTo(PSeat seat);
    static QList<SeatCoord> knightMoveTo(PSeat seat);
    static QList<SeatCoord> rookCannonMoveTo(PSeat seat);
    static QList<SeatCoord> pawnMoveTo(PSeat seat, SeatSide homeSide);

    QList<SeatCoord> bishopRuleFilter(PSeat seat, QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> knightRuleFilter(PSeat seat, QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> rookRuleFilter(QList<SeatCoord>& seatCoordList) const;
    QList<SeatCoord> cannonRuleFilter(QList<SeatCoord>& seatCoordList) const;

private:
    static int symmetryRow_(int row) { return SEATROW - 1 - row; }
    static int symmetryCol_(int col) { return SEATCOL - 1 - col; }

    static QList<SeatCoord>& getValidSeatCoord_(QList<SeatCoord>& seatCoordList,
        bool (*isValidFunc)(SeatCoord));
    static bool isValidSeatCoord_(SeatCoord seatCoord);
    static bool isValidKingAdvSeatCoord_(SeatCoord seatCoord);
    static bool isValidBishopSeatCoord_(SeatCoord seatCoord);

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

Q_DECLARE_METATYPE(SeatSide)

#endif // SEAT_H
