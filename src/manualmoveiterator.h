#ifndef MANUALMOVEITERATOR_H
#define MANUALMOVEITERATOR_H

class Move;
class ManualMove;

class ManualMoveIterator {
public:
    ManualMoveIterator(ManualMove* aManualMove);

    virtual Move*& next() const;
    virtual Move*& doneNext() const;

    virtual bool hasNext() = 0;
    virtual bool hasDoneNext() = 0;

protected:
    void curMoveDone() const;
    void curMoveUndo(bool has) const;
    void checkFinished(bool has) const;

    bool isOther { false };
    Move* oldCurMove;
    ManualMove* manualMove;
};

class ManualMoveOnlyNextIterator : public ManualMoveIterator {
public:
    using ManualMoveIterator::ManualMoveIterator;

    virtual bool hasNext() override;
    virtual bool hasDoneNext() override;

private:
    bool checkGoNext() const;
};

class ManualMoveFirstNextIterator : public ManualMoveIterator {
public:
    using ManualMoveIterator::ManualMoveIterator;

    virtual bool hasNext() override;
    virtual bool hasDoneNext() override;

private:
    bool checkNextOther();
    bool checkGoNextOther() const;
};

#endif // MANUALMOVEITERATOR_H
