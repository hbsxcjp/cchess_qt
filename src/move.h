#ifndef MOVE_H
#define MOVE_H

#include <QPair>
#include <QString>

class Piece;
enum class PieceColor;
enum class ChangeType;

class Seat;
using Coord = QPair<int, int>;
using CoordPair = QPair<Coord, Coord>;
using SeatPair = QPair<Seat*, Seat*>;

class Board;

class Move {
public:
    static Move* creatRootMove();
    static Move* creatMove(Move* preMove, const Board* board, const SeatPair& seatPair,
        const QString& remark, bool isOther);
    static void deleteMove(Move* move);

    PieceColor color() const;

    Move* preMove() const { return preMove_; }
    Move* nextMove() const { return nextMove_; }
    void setNextMove(Move* move) { nextMove_ = move; }
    Move* otherMove() const { return otherMove_; }
    void setOtherMove(Move* move) { otherMove_ = move; }

    SeatPair seatPair() const { return seatPair_; }
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

    CoordPair coordPair() const;
    QString rowcols() const;
    QString iccs() const;

    void done() const;
    void undo() const;

    bool isRoot() const { return !preMove_; }
    bool isNext() const;
    bool isOther() const;

    // 取得前着的着法
    Move* getPrevMove() const;
    QList<const Move*> getPrevMoves() const;

    // 按某种变换类型变换着法记录
    bool changeLayout(const Board* board, ChangeType ct);

    QString toString() const;

private:
    Move() = default;
    Move(Move* preMove, const SeatPair& seatPair, const QString& zhStr,
        const QString& remark, bool isOther);

    SeatPair seatPair_ {};
    Piece* toPiece_ {};

    Move* preMove_ {};
    Move* nextMove_ {};
    Move* otherMove_ {};

    QString zhStr_ {};
    QString remark_ {}; // 注释

    int nextIndex_ { 0 };
    int otherIndex_ { 0 };
    int CC_ColIndex_ { 0 }; // 图中列位置（需在Instance::setMoves确定）
};

#endif // MOVE_H
