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
    virtual void beforeCheck();
    virtual void afterCheck(bool has);

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
    ManualMoveAppendIterator(ManualMove* manualMove);
    ~ManualMoveAppendIterator();

    bool isEnd() const;

    Move* append_coordPair(const CoordPair& coordPair, const QString& remark, bool hasNext, bool hasOther);
    Move* append_rowcols(const QString& rowcols, const QString& remark, bool hasNext, bool hasOther);
    Move* append_zhStr(const QString& zhStr, const QString& remark, bool hasNext, bool hasOther);
    Move* append_iccsZhStr(const QString& iccsOrZhStr, const QString& remark, bool isPGN_ZH, int endBranchNum, bool hasOther);

private:
    void firstNextHandlePreMove(Move* move, bool hasNext, bool hasOther);
    void firstOtherHandlePreMove(Move* move, int endBranchNum, bool hasOther);
    void backBranchNum(Move* move, bool hasBranch, int endBranchNum);

    bool isOther_;

    QStack<Move*> preMoves_;
    ManualMove* manualMove_;
};

#endif // MANUALMOVEITERATOR_H
