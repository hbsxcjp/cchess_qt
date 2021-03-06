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

extern const QString rootZhStr;

class Move {
public:
    Move() = default;
    Move(Move* preMove, const SeatPair& seatPair, const QString& zhStr,
        const QString& remark, bool isOther);

    static void deleteMove(Move* move);

    PieceColor color() const;
    PieceColor color_done() const;

    Move* preMove() const { return preMove_; }
    Move*& nextMove() { return nextMove_; }
    Move*& otherMove() { return otherMove_; }
    void setNextMove(Move* move);
    void setOtherMove(Move* move);
    void insertOtherMove(Move* move);

    SeatPair seatPair() const { return { fromSeat_, toSeat_ }; }
    const QString& zhStr() const { return zhStr_; }

    const QString& remark() const { return remark_; }
    void setRemark(const QString& remark) { remark_ = remark; }

    int nextIndex() const { return nextIndex_; }
    int otherIndex() const { return otherIndex_; }
    int cc_ColIndex() const { return CC_ColIndex_; }
    void setNextIndex();
    void setOtherIndex();
    void setCC_ColIndex(int CC_ColIndex) { CC_ColIndex_ = CC_ColIndex; }

    CoordPair coordPair() const;
    QString rowcols() const;
    QString iccs() const;

    void done();
    void undo();

    bool hasNext() const { return nextMove_; }
    bool hasOther() const { return otherMove_; }
    bool isRoot() const { return !preMove_; }
    bool isLeaf() const { return !hasNext() && !hasOther(); }
    bool isNext() const;
    bool isOther() const;

    // 取得前着的着法
    QList<Move*> getPrevMoves();

    // 按某种变换类型变换着法记录
    bool changeLayout(const Board* board, ChangeType ct);

    QString toString() const;

private:
    Seat* fromSeat_ {};
    Seat* toSeat_ {};
    Piece* toPiece_ {};

    Move* preMove_ {};
    Move* nextMove_ {};
    Move* otherMove_ {};

    QString zhStr_ { rootZhStr };
    QString remark_ {}; // 注释

    int nextIndex_ { 0 };
    int otherIndex_ { 0 };
    int CC_ColIndex_ { 0 }; // 图中列位置（需在Instance::setMoves确定）
};

#endif // MOVE_H
