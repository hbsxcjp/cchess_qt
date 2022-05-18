#ifndef MANUALMOVEITERATOR_H
#define MANUALMOVEITERATOR_H

#include <QList>

class Board;
using Coord = QPair<int, int>;
using CoordPair = QPair<Coord, Coord>;

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

class ManualMoveAppendableIterator {
public:
    ManualMoveAppendableIterator(ManualMove* aManualMove);
    ~ManualMoveAppendableIterator();

    Move* goAppendMove(const Board* board, const CoordPair& coordPair,
        const QString& remark, bool hasNext, bool hasOther);
    Move* goAppendMove(const Board* board, const QString& rowcols,
        const QString& remark, bool hasNext, bool hasOther);
    Move* goAppendMove(const Board* board, const QString& iccsOrZhStr,
        const QString& remark, bool isPGN_ZH, bool hasNext, bool hasOther);
    // 初始化开局库专用
    Move* goAppendMove(const Board* board, const QString& zhStr);

    bool backDeleteMove();

protected:
    void handleOtherPreMove(Move* move, bool hasNext, bool hasOther);

    bool isOther;

    QList<Move*> otherMoves;
    ManualMove* manualMove;
};

#endif // MANUALMOVEITERATOR_H
