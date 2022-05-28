#ifndef BOARD_H
#define BOARD_H

#include <QList>

class Seat;
class BoardSeats;
using Coord = QPair<int, int>;

class Piece;
class BoardPieces;
enum class PieceColor;
enum class PieceKind;

enum class SeatSide;
enum class ChangeType;

using SeatPair = QPair<Seat*, Seat*>;

class Board {
public:
    Board();
    ~Board();

    void init();
    QList<Piece*> getAllPieces() const;
    QList<Seat*> getLiveSeats() const;

    // 测试使用
    Piece* getPiece(Coord coord) const;
    QList<Coord> getLiveSeatCoordList(PieceColor color) const;

    // 棋子可移动位置
    // 1.可移动位置；2.规则已排除位置；3.同色已排除位置；4.将帅对面或被将军已排除位置
    QList<QList<Coord>> canMove(const Coord& fromCoord) const;
    QMap<Seat*, QList<Coord>> allCanMove(PieceColor color) const;
    bool isCanMove(const SeatPair& seatPair) const;

    // 某方棋子是否正在被对方将军
    bool isFace() const;
    bool isKilled(PieceColor color) const;
    bool isFailed(PieceColor color) const;

    QString getPieceChars() const;
    QString getFEN() const;
    bool setFEN(const QString& fen);

    SeatSide getHomeSide(PieceColor color) const;

    SeatPair changeSeatPair(SeatPair seatPair, ChangeType ct) const;
    bool changeLayout(ChangeType ct);

    QString getZhStr(const SeatPair& seatPair) const;
    SeatPair getSeatPair(const QString& zhStr) const;
    SeatPair getSeatPair(const QPair<Coord, Coord>& coordlPair) const;

    QString toString(bool hasEdge = false) const;

private:
    QList<QList<Coord>> canMove(Seat* fromSeat) const;

    QList<Coord> filterKilledRule(Seat* fromSeat, QList<Coord>& coords) const;
    bool isFaceOrKilled(Seat* fromSeat, Seat* toSeat) const;

    bool setBottomColor();

    BoardPieces* boardPieces_;
    BoardSeats* boardSeats_;
    PieceColor bottomColor_;
};

#endif // BOARD_H
