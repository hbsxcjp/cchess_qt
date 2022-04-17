#ifndef MOVE_H
#define MOVE_H

#include <QPair>
#include <QString>

class Seat;
using PSeat = Seat*;

class Piece;
using PPiece = Piece*;
enum class PieceColor;
enum class ChangeType;

class Move;
using PMove = Move*;
using SeatCoord = QPair<int, int>;
using SeatCoordPair = QPair<SeatCoord, SeatCoord>;
using MovSeat = QPair<PSeat, PSeat>;

class Board;
using PBoard = Board*;

class Instance;

class Move {
    friend Instance;

public:
    PieceColor color();

    PMove preMove() const { return preMove_; }
    PMove nextMove() const { return nextMove_; }
    PMove otherMove() const { return otherMove_; }
    MovSeat movSeat() const { return movSeat_; }

    const QString& zhStr() const { return zhStr_; }
    void setZhStr(const QString& zhStr) { zhStr_ = zhStr; }

    const QString& remark() const { return remark_; }
    void setRemark(const QString& remark) { remark_ = remark; }

    int nextIndex() const { return nextIndex_; }
    void setNextIndex(int nextIndex) { nextIndex_ = nextIndex; }
    int otherIndex() const { return otherIndex_; }
    void setOtherIndex(int otherIndex) { otherIndex_ = otherIndex; }
    int cc_ColIndex() const { return CC_ColIndex_; }
    void setCC_ColIndex(int CC_ColIndex) { CC_ColIndex_ = CC_ColIndex; }

    SeatCoordPair seatCoordPair() const;
    QString rowcols() const;
    QString iccs() const;

    void done();
    void undo();

    bool isNext() const;
    bool isOther() const;
    // 取得前着的着法
    PMove getPrevMove();
    QList<PMove> getPrevMoveList();

    PMove addMove(const MovSeat& movSeat, const QString& zhStr, const QString& remark, bool isOther);
    static void deleteMove(PMove move);

    // 按某种变换类型变换着法记录
    bool changeLayout(const PBoard& board, ChangeType ct);

    QString toString() const;

private:
    Move() = default;
    Move(PMove preMove, const MovSeat& movSeat, const QString& zhStr, const QString& remark);

    MovSeat movSeat_ {};
    PPiece toPiece_ {};

    PMove preMove_ {};
    PMove nextMove_ {};
    PMove otherMove_ {};

    QString zhStr_ {};
    QString remark_ {}; // 注释

    int nextIndex_ { 0 };
    int otherIndex_ { 0 };
    int CC_ColIndex_ { 0 }; // 图中列位置（需在Instance::setMoves确定）
};

#endif // MOVE_H
