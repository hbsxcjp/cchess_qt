#ifndef PIECE_H
#define PIECE_H
// 棋子类

#include <QList>
//#include <QMap>
//#include <QPair>
//#include <QString>

class Seat;

enum class PieceColor {
    RED,
    BLACK
};

enum class PieceKind {
    KING,
    ADVISOR,
    BISHOP,
    KNIGHT,
    ROOK,
    CANNON,
    PAWN
};

// 棋子类
class Piece {
public:
    static QList<QList<QList<Piece*>>> creatPieces();

    PieceColor color() const { return color_; }
    PieceKind kind() const { return kind_; }

    bool isLive() const { return seat_; }
    Seat* seat() const { return seat_; }
    void setSeat(Seat* seat) { seat_ = seat; }

    QChar ch() const;
    QChar name() const;
    QChar printName() const;

    QString toString() const;

private:
    Piece(PieceColor color, PieceKind kind);

    const PieceColor color_;
    const PieceKind kind_;

    Seat* seat_;
};

#endif // PIECE_H
