#ifndef MOVE_H
#define MOVE_H

#include <QPair>
#include <QString>
#include <QTextStream>

class Seat;
using PSeat = Seat*;

class Piece;
using PPiece = Piece*;

enum class ChangeType;

class Move;
using PMove = Move*;

using MovSeat = QPair<PSeat, PSeat>;

class Instance;

class Move {
    friend Instance;

public:
    PMove preMove() const { return preMove_; }
    PMove nextMove() const { return nextMove_; }
    PMove otherMove() const { return otherMove_; }

    void done();
    void undo();

    bool isOther();
    // 取得前着的着法
    PMove getPrevMove();
    QList<PMove> getPrevMoves();

    PMove appendMove(const MovSeat& movSeat, const QString& zhStr, const QString& remark, bool isOther);
    void deleteMove(PMove move);

    // 按某种变换类型变换着法记录
    void changeSide(ChangeType ct);

    QString iccs() const;
    QString toString() const;

private:
    Move();
    Move(PMove preMove, MovSeat movSeat, const QString& zhStr, const QString& remark);

    MovSeat movSeat_ {};
    PPiece toPiece_ {};
    PMove preMove_ {}, nextMove_ {}, otherMove_ {};
    QString zhStr_ {}, remark_ {}; // 注释

    int nextNo_ { 0 }, otherNo_ { 0 }, CC_ColNo_ { 0 }; // 图中列位置（需在Instance::setMoves确定）
};

#endif // MOVE_H
