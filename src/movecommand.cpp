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

MoveCommand::MoveCommand(ManualMove* manualMove)
    : manualMove_(manualMove)
{
}

bool GoNextMoveCommand::execute()
{
    return manualMove()->goNext();
}

bool GoNextMoveCommand::unExecute()
{
    return manualMove()->backNext();
}

QString GoNextMoveCommand::exeString() const
{
    return getComString(ComIndex::GoNext, manualMove()->toString());
}

QString GoNextMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackNext, manualMove()->toString());
}

bool BackNextMoveCommand::execute()
{
    return manualMove()->backNext();
}

bool BackNextMoveCommand::unExecute()
{
    return manualMove()->goNext();
}

QString BackNextMoveCommand::exeString() const
{
    return getComString(ComIndex::BackNext, manualMove()->toString());
}

QString BackNextMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoNext, manualMove()->toString());
}

bool GoOtherMoveCommand::execute()
{
    return manualMove()->goOther();
}

bool GoOtherMoveCommand::unExecute()
{
    return manualMove()->backOther();
}

QString GoOtherMoveCommand::exeString() const
{
    return getComString(ComIndex::GoOther, manualMove()->toString());
}

QString GoOtherMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackOther, manualMove()->toString());
}

bool BackOtherMoveCommand::execute()
{
    return manualMove()->backOther();
}

bool BackOtherMoveCommand::unExecute()
{
    return manualMove()->goOther();
}

QString BackOtherMoveCommand::exeString() const
{
    return getComString(ComIndex::BackOther, manualMove()->toString());
}

QString BackOtherMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoOther, manualMove()->toString());
}

static bool goEnd(ManualMove* moveCursor, Move*& endMove)
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

static bool backStart(ManualMove* moveCursor, Move*& endMove)
{
    if (moveCursor->move()->isRoot())
        return false;

    endMove = moveCursor->move();
    return moveCursor->backStart();
}

bool BackToPreMoveCommand::execute()
{
    fromMove_ = manualMove()->move();
    return manualMove()->backAllOtherNext();
}

bool BackToPreMoveCommand::unExecute()
{
    return manualMove()->goToOther(fromMove_);
}

QString BackToPreMoveCommand::exeString() const
{
    return getComString(ComIndex::BackToPre, manualMove()->toString());
}

QString BackToPreMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoToOther, manualMove()->toString());
}

bool GoEndMoveCommand::execute()
{
    return goEnd(manualMove(), endMove_);
}

bool GoEndMoveCommand::unExecute()
{
    return backStart(manualMove(), endMove_);
}

QString GoEndMoveCommand::exeString() const
{
    return getComString(ComIndex::GoEnd, manualMove()->toString());
}

QString GoEndMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackStart, manualMove()->toString());
}

bool BackStartMoveCommand::execute()
{
    return backStart(manualMove(), endMove_);
}

bool BackStartMoveCommand::unExecute()
{
    return goEnd(manualMove(), endMove_);
}

QString BackStartMoveCommand::exeString() const
{
    return getComString(ComIndex::BackStart, manualMove()->toString());
}

QString BackStartMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoEnd, manualMove()->toString());
}

GoToMoveCommand::GoToMoveCommand(ManualMove* movCursor, Move* toMove)
    : MoveCommand(movCursor)
    , fromMove_(movCursor->move())
    , toMove_(toMove)
{
}

bool GoToMoveCommand::execute()
{
    return manualMove()->goTo(toMove_);
}

bool GoToMoveCommand::unExecute()
{
    return manualMove()->goTo(fromMove_);
}

QString GoToMoveCommand::exeString() const
{
    return getComString(ComIndex::GoTo, manualMove()->toString());
}

QString GoToMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackTo, manualMove()->toString());
}

GoIsMoveCommand::GoIsMoveCommand(ManualMove* movCursor, bool isOther)
    : MoveCommand(movCursor)
    , isOther_(isOther)
{
}

bool GoIsMoveCommand::execute()
{
    return manualMove()->goIs(isOther_);
}

bool GoIsMoveCommand::unExecute()
{
    return manualMove()->backIs(isOther_);
}

QString GoIsMoveCommand::exeString() const
{
    return getComString(ComIndex::GoIs, manualMove()->toString());
}

QString GoIsMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackIs, manualMove()->toString());
}

GoIncMoveCommand::GoIncMoveCommand(ManualMove* movCursor, int count)
    : MoveCommand(movCursor)
    , count_(count)
{
}

bool GoIncMoveCommand::execute()
{
    return manualMove()->goInc(count_);
}

bool GoIncMoveCommand::unExecute()
{
    return manualMove()->backInc(count_);
}

QString GoIncMoveCommand::exeString() const
{
    return getComString(ComIndex::GoInc, manualMove()->toString());
}

QString GoIncMoveCommand::unExeString() const
{
    return getComString(ComIndex::BackInc, manualMove()->toString());
}

BackIncMoveCommand::BackIncMoveCommand(ManualMove* movCursor, int count)
    : MoveCommand(movCursor)
    , count_(count)
{
}

bool BackIncMoveCommand::execute()
{
    return manualMove()->backInc(count_);
}

bool BackIncMoveCommand::unExecute()
{
    return manualMove()->goInc(count_);
}

QString BackIncMoveCommand::exeString() const
{
    return getComString(ComIndex::BackInc, manualMove()->toString());
}

QString BackIncMoveCommand::unExeString() const
{
    return getComString(ComIndex::GoInc, manualMove()->toString());
}
