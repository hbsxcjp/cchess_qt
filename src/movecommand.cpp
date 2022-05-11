#include "movecommand.h"
#include "manualmove.h"
#include "move.h"

enum ComIndex {
    GoNext,
    BackNext,
    GoOther,
    BackOther,
    GoToOther,
    BackToPre,
    GoEnd,
    BackStart,
    GoTo,
    BackTo,
    GoIs,
    BackIs,
    GoInc,
    BackInc
};

const QStringList comStrs {
    "前进",
    "后退",
    "进变着",
    "退变着",
    "进至变着",
    "退至前着",
    "进至底",
    "退至始",
    "进至...",
    "退至...",
    "前进(变)",
    "后退(变)",
    "前进...",
    "后退...",
};

QString getComString(int index, const QString& comStr)
{
    return QString("%1 %2").arg(comStrs.at(index), comStr);
}

MoveCommand::MoveCommand(ManualMove*& movCursor)
    : moveCursor_(movCursor)
{
}

bool GoNextMoveCommand::execute()
{
    return moveCursor()->goNext();
}

bool GoNextMoveCommand::unExecute()
{
    return moveCursor()->backNext();
}

QString GoNextMoveCommand::exeString() const
{
    return getComString(ComIndex::GoNext, moveCursor()->toString());
}

QString GoNextMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackNext, moveCursor()->toString());
}

bool BackNextMoveCommand::execute()
{
    return moveCursor()->backNext();
}

bool BackNextMoveCommand::unExecute()
{
    return moveCursor()->goNext();
}

QString BackNextMoveCommand::exeString() const
{
    return getComString(ComIndex::BackNext, moveCursor()->toString());
}

QString BackNextMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoNext, moveCursor()->toString());
}

bool GoOtherMoveCommand::execute()
{
    return moveCursor()->goOther();
}

bool GoOtherMoveCommand::unExecute()
{
    return moveCursor()->backOther();
}

QString GoOtherMoveCommand::exeString() const
{
    return getComString(ComIndex::GoOther, moveCursor()->toString());
}

QString GoOtherMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackOther, moveCursor()->toString());
}

bool BackOtherMoveCommand::execute()
{
    return moveCursor()->backOther();
}

bool BackOtherMoveCommand::unExecute()
{
    return moveCursor()->goOther();
}

QString BackOtherMoveCommand::exeString() const
{
    return getComString(ComIndex::BackOther, moveCursor()->toString());
}

QString BackOtherMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoOther, moveCursor()->toString());
}

static bool goEnd(ManualMove*& moveCursor, Move*& endMove)
{
    bool moved { false };
    if (endMove)
        moved = moveCursor->goTo(endMove);
    else {
        if (!moveCursor->move()->hasNext())
            return false;

        moved = moveCursor->goEnd();
        endMove = moveCursor->move();
    }

    return moved;
}

static bool backStart(ManualMove*& moveCursor, Move*& endMove)
{
    if (moveCursor->move()->isRoot())
        return false;

    endMove = moveCursor->move();
    return moveCursor->backStart();
}

bool BackToPreMoveCommand::execute()
{
    fromMove_ = moveCursor()->move();
    return moveCursor()->backToPre();
}

bool BackToPreMoveCommand::unExecute()
{
    return moveCursor()->goToOther(fromMove_);
}

QString BackToPreMoveCommand::exeString() const
{
    return getComString(ComIndex::BackToPre, moveCursor()->toString());
}

QString BackToPreMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoToOther, moveCursor()->toString());
}

bool GoEndMoveCommand::execute()
{
    return goEnd(moveCursor(), endMove_);
}

bool GoEndMoveCommand::unExecute()
{
    return backStart(moveCursor(), endMove_);
}

QString GoEndMoveCommand::exeString() const
{
    return getComString(ComIndex::GoEnd, moveCursor()->toString());
}

QString GoEndMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackStart, moveCursor()->toString());
}

bool BackStartMoveCommand::execute()
{
    return backStart(moveCursor(), endMove_);
}

bool BackStartMoveCommand::unExecute()
{
    return goEnd(moveCursor(), endMove_);
}

QString BackStartMoveCommand::exeString() const
{
    return getComString(ComIndex::BackStart, moveCursor()->toString());
}

QString BackStartMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoEnd, moveCursor()->toString());
}

GoToMoveCommand::GoToMoveCommand(ManualMove*& movCursor, Move* toMove)
    : MoveCommand(movCursor)
    , fromMove_(movCursor->move())
    , toMove_(toMove)
{
}

bool GoToMoveCommand::execute()
{
    return moveCursor()->goTo(toMove_);
}

bool GoToMoveCommand::unExecute()
{
    return moveCursor()->goTo(fromMove_);
}

QString GoToMoveCommand::exeString() const
{
    return getComString(ComIndex::GoTo, moveCursor()->toString());
}

QString GoToMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackTo, moveCursor()->toString());
}

GoIsMoveCommand::GoIsMoveCommand(ManualMove*& movCursor, bool isOther)
    : MoveCommand(movCursor)
    , isOther_(isOther)
{
}

bool GoIsMoveCommand::execute()
{
    return moveCursor()->goIs(isOther_);
}

bool GoIsMoveCommand::unExecute()
{
    return moveCursor()->backIs(isOther_);
}

QString GoIsMoveCommand::exeString() const
{
    return getComString(ComIndex::GoIs, moveCursor()->toString());
}

QString GoIsMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackIs, moveCursor()->toString());
}

GoIncMoveCommand::GoIncMoveCommand(ManualMove*& movCursor, int count)
    : MoveCommand(movCursor)
    , count_(count)
{
}

bool GoIncMoveCommand::execute()
{
    return moveCursor()->goInc(count_);
}

bool GoIncMoveCommand::unExecute()
{
    return moveCursor()->backInc(count_);
}

QString GoIncMoveCommand::exeString() const
{
    return getComString(ComIndex::GoInc, moveCursor()->toString());
}

QString GoIncMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackInc, moveCursor()->toString());
}

BackIncMoveCommand::BackIncMoveCommand(ManualMove*& movCursor, int count)
    : MoveCommand(movCursor)
    , count_(count)
{
}

bool BackIncMoveCommand::execute()
{
    return moveCursor()->backInc(count_);
}

bool BackIncMoveCommand::unExecute()
{
    return moveCursor()->goInc(count_);
}

QString BackIncMoveCommand::exeString() const
{
    return getComString(ComIndex::BackInc, moveCursor()->toString());
}

QString BackIncMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoInc, moveCursor()->toString());
}
