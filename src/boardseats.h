#ifndef BOARDSEATS_H
#define BOARDSEATS_H

#include <QList>

class Seat;
using Coord = QPair<int, int>;

class Piece;
class BoardPieces;
enum class PieceColor;
enum class PieceKind;

enum class ChangeType;

// 一副棋盘位置类
class BoardSeats {
public:
    BoardSeats();
    ~BoardSeats();

    void clear();
    Seat* getSeat(int index) const;
    Seat* getSeat(const Coord& coord) const;

    Seat* changeSeat(const Seat* seat, ChangeType ct) const;
    void changeLayout(const BoardPieces* boardPieces, ChangeType ct);

    QString getPieceChars() const;
    bool setPieceChars(const BoardPieces* boardPieces, const QString& pieceChars);

    QString getFEN() const;
    bool setFEN(const BoardPieces* boardPieces, const QString& fen);
    bool isFace(Seat* redSeat, Seat* blackSeat) const;

    QString toString(PieceColor bottomColor, bool hasEdge) const;

private:
    QList<Seat*> seats_ {};
};

#endif // BOARDSEATS_H
