#ifndef MANUALMOVEITERATOR_H
#define MANUALMOVEITERATOR_H

#include <QList>

class Seat;
class Board;
using SeatPair = QPair<Seat*, Seat*>;

class Move;
class ManualMove;

class ManualMoveIterator {
public:
    ManualMoveIterator(ManualMove* aManualMove);
    virtual ~ManualMoveIterator() = default;

    void reset();

    bool hasNext();
    Move*& next() const;

protected:
    virtual bool checkBehind() = 0;
    void beforeUse(bool has);
    void afterUsed();

    bool isOther;
    bool firstCheck;

    Move* oldCurMove;
    ManualMove* manualMove;
};

class ManualMoveOnlyNextIterator : public ManualMoveIterator {
public:
    using ManualMoveIterator::ManualMoveIterator;

protected:
    virtual bool checkBehind();
};

class ManualMoveFirstNextIterator : public ManualMoveIterator {
public:
    using ManualMoveIterator::ManualMoveIterator;

protected:
    virtual bool checkBehind();
};

class ManualMoveFirstOtherIterator : public ManualMoveIterator {
public:
    using ManualMoveIterator::ManualMoveIterator;

protected:
    virtual bool checkBehind();
};

// 倒序遍历，从叶子节点至根节点
class ManualMoveReverseIterator : public ManualMoveIterator {
public:
    using ManualMoveIterator::ManualMoveIterator;

protected:
    virtual bool checkBehind();
};

class ManualMoveMutableIterator {
public:
    ManualMoveMutableIterator(ManualMove* aManualMove);

    bool appendMove(const Board* board, const SeatPair& seatPair, const QString& remark, bool isOther);
    bool backDeleteMove();

protected:
    //    bool isOther { false };

    //    Move* oldCurMove;
    ManualMove* manualMove;
};

#endif // MANUALMOVEITERATOR_H
