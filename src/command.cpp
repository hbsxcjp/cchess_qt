#include "command.h"
#include "board.h"
#include "manual.h"
#include "manualmove.h"
#include "move.h"
#include "piece.h"

const QStringList comStrs {
    //
    "放入",
    "取出",

    //
    "增加",
    "删除",

    //
    "前进",
    "后退",
    "进变着",
    "退变着",
    "退至前着",
    "进至底",
    "退至始",
    "进至...",
    "前进...",
    "后退...",
};

QString Command::string() const
{
    return QString("%1\t%2").arg(comStrs.at(index), caption_);
}

PutCommand::PutCommand(Manual* manual, const Coord& coord)
    : coord_(coord)
    , piece_(Q_NULLPTR)
    , board_(manual->board())
{
}

PlacePutCommand::PlacePutCommand(Manual* manual, Piece* piece, const Coord& coord)
    : PutCommand(manual, coord)
{
    piece_ = piece;
    index = ComIndex::PlacePiece;
    caption_ = piece_->name();
}

bool PlacePutCommand::execute()
{
    board_->placePiece(piece_, coord_);
    return true;
}

bool PlacePutCommand::unExecute()
{
    board_->takeOutPiece(coord_);
    return true;
}

TakeOutPutCommand::TakeOutPutCommand(Manual* manual, const Coord& coord)
    : PutCommand(manual, coord)
{
    index = ComIndex::TakeOutPiece;
}

bool TakeOutPutCommand::execute()
{
    piece_ = board_->takeOutPiece(coord_);
    caption_ = piece_->name();
    return true;
}

bool TakeOutPutCommand::unExecute()
{
    board_->placePiece(piece_, coord_);
    return true;
}

MoveCommand::MoveCommand(Manual* manual)
    : manualMove_(manual->manualMove())
    , curMove_(manualMove_->move())
{
}

bool MoveCommand::execute()
{
    manualMove_->goTo(curMove_);
    bool success = coreExecute();
    doneCurMove_ = manualMove_->move();
    caption_ = doneCurMove_->zhStr();

    return success;
}

bool MoveCommand::unExecute()
{
    manualMove_->goTo(doneCurMove_);
    bool success = unCoreExecute();
    Q_ASSERT(curMove_ == manualMove_->move());

    return success;
}

MoveModifyCommand::MoveModifyCommand(Manual* manual)
    : MoveCommand(manual)
{
}

MoveModifyCommand::~MoveModifyCommand()
{
    Move::deleteMove(markDeletedMove_);
}

bool MoveModifyCommand::coreExecute()
{
    bool success = doCoreExecute();
    if (success)
        manualMove_->setNumValues();

    return success;
}

bool MoveModifyCommand::unCoreExecute()
{
    bool success = unDoCoreExecute();
    if (success)
        manualMove_->setNumValues();

    return success;
}

bool MoveModifyCommand::markDeleteMove()
{
    return (markDeletedMove_ = manualMove_->markDeleteCurMove(isOther_, markDeletedMove_));
}

bool MoveModifyCommand::removeMarkDeleteMove()
{
    if (!markDeletedMove_)
        return false;

    if (isOther_) {
        manualMove_->move()->insertOtherMove(markDeletedMove_);
    } else
        manualMove_->move()->setNextMove(markDeletedMove_);

    manualMove_->goIs(isOther_);
    markDeletedMove_ = Q_NULLPTR;

    return true;
}

AppendModifyCommand::AppendModifyCommand(Manual* manual, const CoordPair& coordPair)
    : MoveModifyCommand(manual)
{
    coordPair_ = coordPair;
    index = ComIndex::AppendMove;
}

bool AppendModifyCommand::doCoreExecute()
{
    if (!removeMarkDeleteMove()) {
        Move* oldOtherMove { manualMove_->move()->otherMove() };
        Move* oldNextMove { manualMove_->move()->nextMove() };
        Move* curMove = manualMove_->append_coordPair(coordPair_, "");
        if (!curMove)
            return false;

        if (curMove->isOther())
            curMove->setOtherMove(oldOtherMove);
        else
            markDeletedMove_ = oldNextMove;
    }

    return true;
}

bool AppendModifyCommand::unDoCoreExecute()
{
    return markDeleteMove();
}

DeleteModifyCommand::DeleteModifyCommand(Manual* manual)
    : MoveModifyCommand(manual)
{
    index = ComIndex::DeleteMove;
}

bool DeleteModifyCommand::doCoreExecute()
{
    return markDeleteMove();
}

bool DeleteModifyCommand::unDoCoreExecute()
{
    return removeMarkDeleteMove();
}

GoNextMoveCommand::GoNextMoveCommand(Manual* manual)
    : MoveWalkCommand(manual)
{
    index = ComIndex::GoNext;
}

bool GoNextMoveCommand::coreExecute()
{
    return manualMove_->goNext();
}

bool GoNextMoveCommand::unCoreExecute()
{
    return manualMove_->backNext();
}

BackNextMoveCommand::BackNextMoveCommand(Manual* manual)
    : MoveWalkCommand(manual)
{
    index = ComIndex::BackNext;
}

bool BackNextMoveCommand::coreExecute()
{
    return manualMove_->backNext();
}

bool BackNextMoveCommand::unCoreExecute()
{
    return manualMove_->goNext();
}

GoOtherMoveCommand::GoOtherMoveCommand(Manual* manual)
    : MoveWalkCommand(manual)
{
    index = ComIndex::GoOther;
}

bool GoOtherMoveCommand::coreExecute()
{
    return manualMove_->goOther();
}

bool GoOtherMoveCommand::unCoreExecute()
{
    return manualMove_->backOther();
}

BackOtherMoveCommand::BackOtherMoveCommand(Manual* manual)
    : MoveWalkCommand(manual)
{
    index = ComIndex::BackOther;
}

bool BackOtherMoveCommand::coreExecute()
{
    return manualMove_->backOther();
}

bool BackOtherMoveCommand::unCoreExecute()
{
    return manualMove_->goOther();
}

BackToPreMoveCommand::BackToPreMoveCommand(Manual* manual)
    : MoveWalkCommand(manual)
{
    index = ComIndex::BackToPre;
}

bool BackToPreMoveCommand::coreExecute()
{
    return manualMove_->backAllOtherNext();
}

bool BackToPreMoveCommand::unCoreExecute()
{
    return manualMove_->goToOther(curMove_);
}

GoEndMoveCommand::GoEndMoveCommand(Manual* manual)
    : MoveWalkCommand(manual)
{
    index = ComIndex::GoEnd;
}

bool GoEndMoveCommand::coreExecute()
{
    return manualMove_->goEnd();
}

bool GoEndMoveCommand::unCoreExecute()
{
    return manualMove_->goTo(curMove_);
}

BackStartMoveCommand::BackStartMoveCommand(Manual* manual)
    : MoveWalkCommand(manual)
{
    index = ComIndex::BackStart;
}

bool BackStartMoveCommand::coreExecute()
{
    return manualMove_->backStart();
}

bool BackStartMoveCommand::unCoreExecute()
{
    return manualMove_->goTo(curMove_);
}

GoToMoveCommand::GoToMoveCommand(Manual* manual, Move* toMove)
    : MoveWalkCommand(manual)
{
    doneCurMove_ = toMove;
    index = ComIndex::GoTo;
}

bool GoToMoveCommand::coreExecute()
{
    return manualMove_->goTo(doneCurMove_);
}

bool GoToMoveCommand::unCoreExecute()
{
    return manualMove_->goTo(curMove_);
}

GoIncMoveCommand::GoIncMoveCommand(Manual* manual, int count)
    : MoveWalkCommand(manual)
    , count_(count)
{
    index = ComIndex::GoInc;
}

bool GoIncMoveCommand::coreExecute()
{
    return manualMove_->goInc(count_);
}

bool GoIncMoveCommand::unCoreExecute()
{
    return manualMove_->backInc(count_);
}

BackIncMoveCommand::BackIncMoveCommand(Manual* manual, int count)
    : MoveWalkCommand(manual)
    , count_(count)
{
    index = ComIndex::BackInc;
}

bool BackIncMoveCommand::coreExecute()
{
    return manualMove_->backInc(count_);
}

bool BackIncMoveCommand::unCoreExecute()
{
    return manualMove_->goInc(count_);
}

CommandContainer::CommandContainer()
    : revokeCommands({})
    , recoverCommands({})
{
}

CommandContainer::~CommandContainer()
{
    clear();
}

bool CommandContainer::append(Command* command, bool allowPush)
{
    bool success = command->execute();
    if (success) {
        if (allowPush)
            revokeCommands.push(command);
    } else
        delete command;

    return success;
}

bool CommandContainer::revoke(int num, CommandType& type)
{
    bool success { false };
    while (num-- > 0 && !revokeCommands.isEmpty()) {
        Command* command = revokeCommands.pop();
        recoverCommands.push(command);
        type = command->type();
        success = command->unExecute();
    }

    return success;
}

bool CommandContainer::recover(int num, CommandType& type)
{
    bool success { false };
    while (num-- > 0 && !recoverCommands.isEmpty()) {
        Command* command = recoverCommands.pop();
        revokeCommands.push(command);
        type = command->type();
        success = command->execute();
    }

    return success;
}

void CommandContainer::clear()
{
    clearRevokes();
    clearRecovers();
}

void CommandContainer::clearRevokes()
{
    while (!revokeCommands.isEmpty())
        delete revokeCommands.pop();
}

void CommandContainer::clearRecovers()
{
    while (!recoverCommands.isEmpty())
        delete recoverCommands.pop();
}

QStringList CommandContainer::getRevokeStrings() const
{
    QStringList strings;
    int index = 0;
    for (auto& command : revokeCommands.toList())
        strings.prepend(QString("%1. %2").arg(index++).arg(command->string()));

    return strings;
}

QStringList CommandContainer::getRecoverStrings() const
{
    QStringList strings;
    int index = revokeCommands.size() + recoverCommands.size() - 1;
    for (auto& command : recoverCommands.toList())
        strings.prepend(QString("%1. %2").arg(index--).arg(command->string()));

    return strings;
}
