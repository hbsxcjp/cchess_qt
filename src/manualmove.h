#ifndef MANUALMOVE_H
#define MANUALMOVE_H

#include <QList>

enum class PieceColor;

class Seat;
class Board;
class Move;
using SeatPair = QPair<Seat*, Seat*>;

// 着法游标（操作）类
class ManualMove {
public:
    ManualMove();
    ~ManualMove();

    Move*& rootMove() { return rootMove_; }
    Move*& move() { return curMove; }
    Move* appendMove(const Board* board, const SeatPair& seatPair, const QString& remark, bool isOther);

    bool isEmpty() const;
    PieceColor firstColor() const;

    void setCurMove(Move*& move);

    bool goNext(); // 前进
    bool backNext(); // 本着非变着，则回退一着

    bool goOther(); // 前进变着
    bool backOther(); // 回退变着

    bool goToOther(Move* otherMove); // 前进至指定变着
    bool backToPre(); // 变着回退至前着

    bool goEnd(); // 前进至底
    bool backStart(); // 回退至首着

    bool goTo(Move* move); // 前进至指定move

    bool goIs(bool isOther);
    bool backIs(bool isOther);

    bool goInc(int inc); // 前进数步
    bool backInc(int inc); // 后退数步

    QString toString() const;

private:
    Move* rootMove_;
    Move* curMove;
};

#endif // MANUALMOVE_H