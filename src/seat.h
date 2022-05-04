#ifndef SEAT_H
#define SEAT_H

#include <QList>

class SeatBase;
class BoardSeats;
enum class SeatSide;
using Coord = QPair<int, int>;

class Piece;

// 位置类
class Seat {
public:
    static QList<Seat*> creatSeats();

    int row() const { return coord_.first; }
    int col() const { return coord_.second; }
    Coord coord() const { return coord_; }
    bool isBottom() const;

    bool hasPiece() const { return piece_; }
    Piece* piece() const { return piece_; }
    void setPiece(Piece* piece);

    void moveTo(Seat* toSeat, Piece* fillPiece = Q_NULLPTR);

    // 棋子从某位置可移至位置(排除不符合走棋规则的位置, 排除目标同色的位置)
    // 1.可移动位置；2.规则已排除位置；3.同色已排除位置
    QList<QList<Coord>> canMove(const BoardSeats* boardSeats, SeatSide homeSide) const;

    QString toString() const;

private:
    Seat(const Coord& coord);

    Piece* piece_;
    const Coord coord_;
};

#endif // SEAT_H
