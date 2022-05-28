#include "movecommand.h"
#include "manual.h"
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
    "前进...",
    "后退...",
};

static QString getShowString(int index, const QString& zhStr)
{
    return QString("%1\t%2").arg(comStrs.at(index), zhStr);
}

MoveCommand::MoveCommand(Manual* manual)
    : manualMove_(manual->manualMove())
    , fromMove_(manualMove_->move())
{
}

bool GoNextMoveCommand::execute()
{
    bool success = manualMove_->goNext();
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool GoNextMoveCommand::unExecute()
{
    return manualMove_->backNext();
}

QString GoNextMoveCommand::string() const
{
    return getShowString(ComIndex::GoNext, curZhStr);
}

bool BackNextMoveCommand::execute()
{
    bool success = manualMove_->backNext();
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool BackNextMoveCommand::unExecute()
{
    return manualMove_->goNext();
}

QString BackNextMoveCommand::string() const
{
    return getShowString(ComIndex::BackNext, curZhStr);
}

bool GoOtherMoveCommand::execute()
{
    bool success = manualMove_->goOther();
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool GoOtherMoveCommand::unExecute()
{
    return manualMove_->backOther();
}

QString GoOtherMoveCommand::string() const
{
    return getShowString(ComIndex::GoOther, curZhStr);
}

bool BackOtherMoveCommand::execute()
{
    bool success = manualMove_->backOther();
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool BackOtherMoveCommand::unExecute()
{
    return manualMove_->goOther();
}

QString BackOtherMoveCommand::string() const
{
    return getShowString(ComIndex::BackOther, curZhStr);
}

bool BackToPreMoveCommand::execute()
{
    bool success = manualMove_->backAllOtherNext();
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool BackToPreMoveCommand::unExecute()
{
    return manualMove_->goToOther(fromMove_);
}

QString BackToPreMoveCommand::string() const
{
    return getShowString(ComIndex::BackToPre, curZhStr);
}

bool GoEndMoveCommand::execute()
{
    bool success = manualMove_->goEnd();
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool GoEndMoveCommand::unExecute()
{
    return manualMove_->goTo(fromMove_);
}

QString GoEndMoveCommand::string() const
{
    return getShowString(ComIndex::GoEnd, curZhStr);
}

bool BackStartMoveCommand::execute()
{
    bool success = manualMove_->backStart();
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool BackStartMoveCommand::unExecute()
{
    return manualMove_->goTo(fromMove_);
}

QString BackStartMoveCommand::string() const
{
    return getShowString(ComIndex::BackStart, curZhStr);
}

GoToMoveCommand::GoToMoveCommand(Manual* manual, Move* toMove)
    : MoveCommand(manual)
{
    toMove_ = toMove;
}

bool GoToMoveCommand::execute()
{
    bool success = manualMove_->goTo(toMove_);
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool GoToMoveCommand::unExecute()
{
    return manualMove_->goTo(fromMove_);
}

QString GoToMoveCommand::string() const
{
    return getShowString(ComIndex::GoTo, curZhStr);
}

GoIncMoveCommand::GoIncMoveCommand(Manual* manual, int count)
    : MoveCommand(manual)
    , count_(count)
{
}

bool GoIncMoveCommand::execute()
{
    bool success = manualMove_->goInc(count_);
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool GoIncMoveCommand::unExecute()
{
    return manualMove_->backInc(count_);
}

QString GoIncMoveCommand::string() const
{
    return getShowString(ComIndex::GoInc, curZhStr);
}

BackIncMoveCommand::BackIncMoveCommand(Manual* manual, int count)
    : MoveCommand(manual)
    , count_(count)
{
}

bool BackIncMoveCommand::execute()
{
    bool success = manualMove_->backInc(count_);
    curZhStr = manualMove_->curZhStr();
    return success;
}

bool BackIncMoveCommand::unExecute()
{
    return manualMove_->goInc(count_);
}

QString BackIncMoveCommand::string() const
{
    return getShowString(ComIndex::BackInc, curZhStr);
}

MoveCommandContainer::MoveCommandContainer()
    : revokeCommands({})
    , recoverCommands({})
{
}

MoveCommandContainer::~MoveCommandContainer()
{
    clearRevokes();
    clearRecovers();
}

bool MoveCommandContainer::append(MoveCommand* command)
{
    bool success = command->execute();
    if (success) {
        clearRecovers();
        revokeCommands.push(command);
    } else
        delete command;

    return success;
}

QStringList MoveCommandContainer::getRevokeStrings() const
{
    QStringList strings;
    int index = 0;
    for (auto& command : revokeCommands.toList())
        strings.prepend(QString("%1. %2").arg(index++).arg(command->string()));

    return strings;
}

QStringList MoveCommandContainer::getRecoverStrings() const
{
    QStringList strings;
    int index = revokeCommands.size() + recoverCommands.size() - 1;
    for (auto& command : recoverCommands.toList())
        strings.prepend(QString("%1. %2").arg(index--).arg(command->string()));

    return strings;
}

bool MoveCommandContainer::revoke(int num)
{
    bool success { false };
    while (num-- > 0 && !revokeCommands.isEmpty()) {
        MoveCommand* command = revokeCommands.pop();
        recoverCommands.push(command);
        success = command->unExecute();
    }

    return success;
}

bool MoveCommandContainer::recover(int num)
{
    bool success { false };
    while (num-- > 0 && !recoverCommands.isEmpty()) {
        MoveCommand* command = recoverCommands.pop();
        revokeCommands.push(command);
        success = command->execute();
    }

    return success;
}

void MoveCommandContainer::clearRevokes()
{
    while (!revokeCommands.isEmpty())
        delete revokeCommands.pop();
}

void MoveCommandContainer::clearRecovers()
{
    while (!recoverCommands.isEmpty())
        delete recoverCommands.pop();
}
