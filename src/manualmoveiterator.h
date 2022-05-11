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

    virtual bool hasNext() = 0;
    virtual Move*& next() const;

protected:
    bool isOther { false };
    Move* oldCurMove;
    ManualMove* manualMove;
};

class ManualMoveOnlyNextIterator : public ManualMoveIterator {
public:
    using ManualMoveIterator::ManualMoveIterator;

    virtual bool hasNext() override;
};

class ManualMoveFirstNextIterator : public ManualMoveIterator {
public:
    using ManualMoveIterator::ManualMoveIterator;

    virtual bool hasNext() override;

private:
    bool firstCheck { true };

    bool checkNextOther();
};

class ManualMoveMutableIterator {
public:
    ManualMoveMutableIterator(ManualMove* aManualMove);

    bool appendMove(const Board* board, const SeatPair& seatPair, const QString& remark, bool isOther);

protected:
    //    bool isOther { false };

    //    Move* oldCurMove;
    ManualMove* manualMove;
};

#endif // MANUALMOVEITERATOR_H
