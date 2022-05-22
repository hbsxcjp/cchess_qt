#ifndef MANUALMOVEITERATOR_H
#define MANUALMOVEITERATOR_H

#include <QStack>

class Board;
using Coord = QPair<int, int>;
using CoordPair = QPair<Coord, Coord>;

class Move;
class ManualMove;

class Manual;

class ManualMoveIterator {
public:
    ManualMoveIterator(ManualMove* aManualMove);
    virtual ~ManualMoveIterator() = default;

    void reset();

    bool hasNext();
    Move*& next() const;

protected:
    virtual bool checkBehind() = 0;
    virtual void beforeUse(bool has);
    virtual void afterUsed();

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

class ManualMoveAppendIterator {
public:
    ManualMoveAppendIterator(Manual* manual);
    ~ManualMoveAppendIterator();

    bool isEnd() const;

    Move* appendGo(const CoordPair& coordPair, const QString& remark, bool hasNext, bool hasOther);
    Move* appendGo(const QString& rowcols, const QString& remark, bool hasNext, bool hasOther);
    Move* appendGo(const QString& iccsOrZhStr,
        const QString& remark, bool isPGN_ZH, bool hasNext, bool hasOther);
    Move* appendGo(const QString& iccsOrZhStr,
        const QString& remark, bool isPGN_ZH, int endBranchNum, bool hasOther);

    bool backDeleteMove();

protected:
    void firstNextHandlePreMove(Move* move, bool hasNext, bool hasOther);
    void firstOtherHandlePreMove(Move* move, int endBranchNum, bool hasOther);

    bool isOther_;

    const Board* board;
    QStack<Move*> preMoves;
    ManualMove* manualMove;
};

#endif // MANUALMOVEITERATOR_H
