#ifndef MOVE_H
#define MOVE_H

#include <QPair>
#include <QString>

class Seat;
class Seats;
using PSeat = Seat*;

class Piece;
using PPiece = Piece*;

class Pieces;

class Move;
using PMove = Move*;

using MovSeat = QPair<PSeat, PSeat>;

class Instance;

class Move {
    friend Instance;

public:
    Move(PMove preMove, MovSeat movSeat);

    PMove addMove(MovSeat movSeat, bool isOther);
    void deleteMove(PMove move);

    PMove nextMove() const { return nextMove_; }
    PMove otherMove() const { return otherMove_; }

    void setNextMove(PMove nextMove) { nextMove_ = nextMove; }
    void setOtherMove(PMove otherMove) { otherMove_ = otherMove; }

    void done();
    void undo();

private:
    Move();

    MovSeat movSeat_;
    PPiece toPiece_;

    PMove preMove_ {}, nextMove_ {}, otherMove_ {};

    QString zhStr_ {}, remark_ {}; // 注释
    int nextNo_ { 0 }, otherNo_ { 0 }, CC_ColNo_ { 0 }; // 图中列位置（需在Instance::setMoves确定）
};

#endif // MOVE_H
