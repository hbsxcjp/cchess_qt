#ifndef SEATBASE_H
#define SEATBASE_H

#include <QList>
#include <QMetaType>

class Piece;
enum class PieceColor;
enum class PieceKind;

class BoardSeats;
using Coord = QPair<int, int>;

class Seat;
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

// 位置坐标类
class SeatBase {
public:
    static QList<Coord> allCoord();
    static int getRowNum();
    static int getColNum();
    static int getSeatNum();

    static bool isBottom(const Coord& coord);
    static Coord getCoord(int index);
    static int getIndex(const Coord& coord);

    static int symmetryRow(int row);
    static int symmetryCol(int col);
    static Coord changeCoord(const Coord& coord, ChangeType ct);

    static QString pieCharsToFEN(const QString& pieChars);
    static QString FENToPieChars(const QString& fen);
    static const QList<QPair<Coord, QPair<PieceColor, PieceKind>>>& getInitCoordColorKinds();

    static QPair<Coord, Coord> coordPair(const QString& rowcols);
    static bool less(Seat* first, Seat* last);

    // 棋子可置入位置坐标
    static QList<Coord> canPut(PieceKind kind, SeatSide homeSide);

    // 在某位置处棋子可移动所至的位置坐标
    static QList<QList<Coord>> canMove(Piece* piece, const Coord& coord,
        const BoardSeats* boardSeats, SeatSide homeSide);

private:
    static QList<Coord> getCanMove(PieceKind kind, const Coord& coord, SeatSide homeSide);
    static QList<Coord> filterMoveRule(const BoardSeats* boardSeats, PieceKind kind,
        const Coord& coord, QList<Coord>& coords);
    static QList<Coord> filterColorRule(const BoardSeats* boardSeats, PieceColor color, QList<Coord>& coords);

    static QList<Coord> filterBishopMoveRule(const BoardSeats* boardSeats,
        const Coord& coord, QList<Coord>& coords);
    static QList<Coord> filterKnightMoveRule(const BoardSeats* boardSeats,
        const Coord& coord, QList<Coord>& coords);
    static QList<Coord> filterRookMoveRule(const BoardSeats* boardSeats, QList<Coord>& coords);
    static QList<Coord> filterCannonMoveRule(const BoardSeats* boardSeats, QList<Coord>& coords);

    static QList<Coord> kingCanPut(SeatSide homeSide);
    static QList<Coord> advisorCanPut(SeatSide homeSide);
    static QList<Coord> bishopCanPut(SeatSide homeSide);
    static QList<Coord> pawnCanPut(SeatSide homeSide);

    static QList<Coord> kingCanMove(int row, int col);
    static QList<Coord> advisorCanMove(int row, int col, SeatSide homeSide);
    static QList<Coord> bishopCanMove(int row, int col);
    static QList<Coord> knightCanMove(int row, int col);
    static QList<Coord> rookCannonCanMove(int fromRow, int fromCol);
    static QList<Coord> pawnCanMove(int row, int col, SeatSide homeSide);

    static QList<Coord>& filterValid(QList<Coord>& coords, bool (*isFunc)(const Coord&));
    static bool isKingAdvCoord(const Coord& coord);
    static bool isBishopCoord(const Coord& coord);

    static bool isValid(const Coord& coord);
    static bool isRow(int row);
    static bool isCol(int col);

    static bool isKingAdvRow(int row);
    static bool isKingAdvCol(int col);
    static bool isBishopRow(int row);
};

QList<Coord> getCoordList(const QList<Seat*>& seats);

QString getCoordString(const Coord& coord);
QString getCoordListString(const QList<Coord>& coords);

QString getSeatListString(const QList<Seat*>& seats);

Q_DECLARE_METATYPE(SeatSide)

#endif // SEATBASE_H
