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

static QString getShowString(int index, const QString& comStr)
{
    return QString("%1 %2").arg(comStrs.at(index), comStr);
}

MoveCommand::MoveCommand(ManualMove* manualMove)
    : manualMove_(manualMove)
{
}

bool GoNextMoveCommand::execute()
{
    return manualMove_->goNext();
}

bool GoNextMoveCommand::unExecute()
{
    return manualMove_->backNext();
}

QString GoNextMoveCommand::exeString() const
{
    return getShowString(ComIndex::GoNext, manualMove_->curZhStr());
}

QString GoNextMoveCommand::unExeString() const
{
    return getShowString(ComIndex::BackNext, manualMove_->curZhStr());
}

bool BackNextMoveCommand::execute()
{
    return manualMove_->backNext();
}

bool BackNextMoveCommand::unExecute()
{
    return manualMove_->goNext();
}

QString BackNextMoveCommand::exeString() const
{
    return getShowString(ComIndex::BackNext, manualMove_->curZhStr());
}

QString BackNextMoveCommand::unExeString() const
{
    return getShowString(ComIndex::GoNext, manualMove_->curZhStr());
}

bool GoOtherMoveCommand::execute()
{
    return manualMove_->goOther();
}

bool GoOtherMoveCommand::unExecute()
{
    return manualMove_->backOther();
}

QString GoOtherMoveCommand::exeString() const
{
    return getShowString(ComIndex::GoOther, manualMove_->curZhStr());
}

QString GoOtherMoveCommand::unExeString() const
{
    return getShowString(ComIndex::BackOther, manualMove_->curZhStr());
}

bool BackOtherMoveCommand::execute()
{
    return manualMove_->backOther();
}

bool BackOtherMoveCommand::unExecute()
{
    return manualMove_->goOther();
}

QString BackOtherMoveCommand::exeString() const
{
    return getShowString(ComIndex::BackOther, manualMove_->curZhStr());
}

QString BackOtherMoveCommand::unExeString() const
{
    return getShowString(ComIndex::GoOther, manualMove_->curZhStr());
}

static bool goEnd(ManualMove* manualMove, Move*& endMove)
{
    bool moved { false };
    if (endMove)
        moved = manualMove->goTo(endMove);
    else {
        if (!manualMove->move()->hasNext())
            return false;

        moved = manualMove->goEnd();
        endMove = manualMove->move();
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
    fromMove_ = manualMove_->move();
    return manualMove_->backAllOtherNext();
}

bool BackToPreMoveCommand::unExecute()
{
    return manualMove_->goToOther(fromMove_);
}

QString BackToPreMoveCommand::exeString() const
{
    return getShowString(ComIndex::BackToPre, manualMove_->curZhStr());
}

QString BackToPreMoveCommand::unExeString() const
{
    return getShowString(ComIndex::GoToOther, manualMove_->curZhStr());
}

bool GoEndMoveCommand::execute()
{
    return goEnd(manualMove_, endMove_);
}

bool GoEndMoveCommand::unExecute()
{
    return backStart(manualMove_, endMove_);
}

QString GoEndMoveCommand::exeString() const
{
    return getShowString(ComIndex::GoEnd, manualMove_->curZhStr());
}

QString GoEndMoveCommand::unExeString() const
{
    return getShowString(ComIndex::BackStart, manualMove_->curZhStr());
}

bool BackStartMoveCommand::execute()
{
    return backStart(manualMove_, endMove_);
}

bool BackStartMoveCommand::unExecute()
{
    return goEnd(manualMove_, endMove_);
}

QString BackStartMoveCommand::exeString() const
{
    return getShowString(ComIndex::BackStart, manualMove_->curZhStr());
}

QString BackStartMoveCommand::unExeString() const
{
    return getShowString(ComIndex::GoEnd, manualMove_->curZhStr());
}

GoToMoveCommand::GoToMoveCommand(ManualMove* movCursor, Move* toMove)
    : MoveCommand(movCursor)
    , fromMove_(movCursor->move())
    , toMove_(toMove)
{
}

bool GoToMoveCommand::execute()
{
    return manualMove_->goTo(toMove_);
}

bool GoToMoveCommand::unExecute()
{
    return manualMove_->goTo(fromMove_);
}

QString GoToMoveCommand::exeString() const
{
    return getShowString(ComIndex::GoTo, manualMove_->curZhStr());
}

QString GoToMoveCommand::unExeString() const
{
    return getShowString(ComIndex::BackTo, manualMove_->curZhStr());
}

GoIsMoveCommand::GoIsMoveCommand(ManualMove* movCursor, bool isOther)
    : MoveCommand(movCursor)
    , isOther_(isOther)
{
}

bool GoIsMoveCommand::execute()
{
    return manualMove_->goIs(isOther_);
}

bool GoIsMoveCommand::unExecute()
{
    return manualMove_->backIs(isOther_);
}

QString GoIsMoveCommand::exeString() const
{
    return getShowString(ComIndex::GoIs, manualMove_->curZhStr());
}

QString GoIsMoveCommand::unExeString() const
{
    return getShowString(ComIndex::BackIs, manualMove_->curZhStr());
}

GoIncMoveCommand::GoIncMoveCommand(ManualMove* movCursor, int count)
    : MoveCommand(movCursor)
    , count_(count)
{
}

bool GoIncMoveCommand::execute()
{
    return manualMove_->goInc(count_);
}

bool GoIncMoveCommand::unExecute()
{
    return manualMove_->backInc(count_);
}

QString GoIncMoveCommand::exeString() const
{
    return getShowString(ComIndex::GoInc, manualMove_->curZhStr());
}

QString GoIncMoveCommand::unExeString() const
{
    return getShowString(ComIndex::BackInc, manualMove_->curZhStr());
}

BackIncMoveCommand::BackIncMoveCommand(ManualMove* movCursor, int count)
    : MoveCommand(movCursor)
    , count_(count)
{
}

bool BackIncMoveCommand::execute()
{
    return manualMove_->backInc(count_);
}

bool BackIncMoveCommand::unExecute()
{
    return manualMove_->goInc(count_);
}

QString BackIncMoveCommand::exeString() const
{
    return getShowString(ComIndex::BackInc, manualMove_->curZhStr());
}

QString BackIncMoveCommand::unExeString() const
{
    return getShowString(ComIndex::GoInc, manualMove_->curZhStr());
}

MoveCommandContainer::MoveCommandContainer()
    : revokeCommands({})
    , recoverCommands({})
{
}

MoveCommandContainer::~MoveCommandContainer()
{
    for (auto& commands : QList<QStack<MoveCommand*>> { revokeCommands, recoverCommands })
        while (!commands.isEmpty())
            delete commands.pop();
}

void MoveCommandContainer::append(MoveCommand* command)
{
    if (command->execute())
        revokeCommands.push(command);
    else
        delete command;
}

void MoveCommandContainer::revoke()
{
    MoveCommand* command = moveTop(revokeCommands, recoverCommands);
    if (command)
        command->unExecute();
}

void MoveCommandContainer::recover()
{
    MoveCommand* command = moveTop(recoverCommands, revokeCommands);
    if (command)
        command->execute();
}

MoveCommand* MoveCommandContainer::moveTop(QStack<MoveCommand*>& fromCommands, QStack<MoveCommand*>& toCommands)
{
    MoveCommand* command {};
    if (!fromCommands.isEmpty()) {
        command = fromCommands.pop();
        toCommands.push(command);
    }

    return command;
}
