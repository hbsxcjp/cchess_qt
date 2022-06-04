#ifndef MANUALMOVE_H
#define MANUALMOVE_H

#include <QList>

//#define DEBUG

enum class PieceColor;

class Seat;
class Board;
class Move;
using Coord = QPair<int, int>;
using CoordPair = QPair<Coord, Coord>;
using SeatPair = QPair<Seat*, Seat*>;

// 着法游标（操作）类
class ManualMove {
public:
    ManualMove(const Board* board);
    ~ManualMove();

    Move* append_coordPair(const CoordPair& coordPair, const QString& remark);
    Move* append_rowcols(const QString& rowcols, const QString& remark);
    Move* append_iccs(const QString& iccs, const QString& remark);
    Move* append_zhStr(const QString& zhStr, const QString& remark);

    Move* deleteCurMove(bool& isOther, Move* deletedMove = Q_NULLPTR);

    PieceColor firstColor() const;
    PieceColor curColor() const;

    bool goNext(); // 前进
    bool backNext(); // 本着非变着，则回退一着

    bool goOther(); // 前进变着
    bool backOther(); // 回退变着

    bool goToOther(Move* otherMove); // 前进至指定变着
    bool backAllOtherNext(); // 变着回退至前着
    bool backAllNextOther(); // 后着回退至前着
    bool backNextToHasOther(); // 后着回退至具有变着的前着

    bool goEndPre(); // 前进至底前着
    bool goOtherEndPre(); // 前进至Other底前着
    bool goEnd(); // 前进至底
    bool backStart(); // 回退至首着

    bool goTo(Move* move); // 前进至指定move

    bool goIs(bool isOther);
    bool backIs(bool isOther);
    bool go();
    bool back();

    bool goInc(int inc); // 前进数步
    bool backInc(int inc); // 后退数步

    bool isCurMove(Move* move) const;
    const QString& getCurRemark() const;
    void setCurRemark(const QString& remark) const;

    SeatPair curSeatPair() const;
    CoordPair curCoordPair() const;

    Move*& rootMove() { return rootMove_; }
    Move*& move() { return curMove_; }

    void setMoveNums();
    int getMovCount() const { return movCount_; }
    int getRemCount() const { return remCount_; }
    int getRemLenMax() const { return remLenMax_; }
    int maxRow() const { return maxRow_; }
    int maxCol() const { return maxCol_; }

    QString moveInfo() const;
    QString curZhStr() const;

private:
    Move* append_seatPair(SeatPair seatPair, const QString& remark, QString zhStr = "");

    const Board* board_;
    Move* rootMove_;
    Move* curMove_;

    int movCount_ { 0 };
    int remCount_ { 0 };
    int remLenMax_ { 0 };
    int maxRow_ { 0 };
    int maxCol_ { 0 };
};

#endif // MANUALMOVE_H
