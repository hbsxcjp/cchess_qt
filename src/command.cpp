#include "command.h"
#include "board.h"
#include "manual.h"
#include "manualmove.h"
#include "move.h"
#include "piece.h"

enum ComIndex {
    // 放棋
    PlacePiece,
    TakeOutPiece,

    // 移动
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
    BackInc,

    // 修改
    AppendMove,
    DeleteMove,
};

const QStringList comStrs {
    //
    "放入",
    "取出",

    //
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

    //
    "增加",
    "删除",
};

static QString getShowString(int index, const QString& caption)
{
    return QString("%1\t%2").arg(comStrs.at(index), caption);
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

QString PlacePutCommand::string() const
{
    return getShowString(ComIndex::PlacePiece, piece_->name());
}

bool TakeOutPutCommand::execute()
{
    piece_ = board_->takeOutPiece(coord_);
    return true;
}

bool TakeOutPutCommand::unExecute()
{
    board_->placePiece(piece_, coord_);
    return true;
}

QString TakeOutPutCommand::string() const
{
    return getShowString(ComIndex::TakeOutPiece, piece_->name());
}

MoveCommand::MoveCommand(Manual* manual)
    : manualMove_(manual->manualMove())
    , fromMove_(manualMove_->move())
{
}

bool GoNextMoveCommand::execute()
{
    bool success = manualMove_->goNext();
    if (success)
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
    if (success)
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
    if (success)
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
    if (success)
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
    if (success)
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
    if (success)
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
    if (success)
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
    if (success)
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
    if (success)
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
    if (success)
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

ModifyCommand::ModifyCommand(Manual* manual)
    : manualMove_(manual->manualMove())
{
}

ModifyCommand::~ModifyCommand()
{
    Move::deleteMove(deletedMove_);
}

AppendModifyCommand::AppendModifyCommand(Manual* manual, const CoordPair& coordPair)
    : ModifyCommand(manual)
{
    coordPair_ = coordPair;
}

bool AppendModifyCommand::execute()
{
    Move* oldOtherMove { manualMove_->move()->otherMove() };
    Move* oldNextMove { manualMove_->move()->nextMove() };
    Move* curMove = manualMove_->append_coordPair(coordPair_, "");
    if (!curMove)
        return false;

    curZhStr = manualMove_->curZhStr();
    if (curMove->isOther())
        curMove->setOtherMove(oldOtherMove);
    else
        deletedMove_ = oldNextMove;

    manualMove_->setMoveNums();
    return true;
}

bool AppendModifyCommand::unExecute()
{
    Move::deleteMove(manualMove_->deleteCurMove(isOther_, deletedMove_));
    deletedMove_ = Q_NULLPTR;

    manualMove_->setMoveNums();
    return true;
}

QString AppendModifyCommand::string() const
{
    return getShowString(ComIndex::AppendMove, curZhStr);
}

bool DeleteModifyCommand::execute()
{
    return (deletedMove_ = manualMove_->deleteCurMove(isOther_));
}

bool DeleteModifyCommand::unExecute()
{
    if (isOther_) {
        deletedMove_->setOtherMove(manualMove_->move()->otherMove());
        manualMove_->move()->setOtherMove(deletedMove_);
    } else
        manualMove_->move()->setNextMove(deletedMove_);

    manualMove_->goIs(isOther_);
    deletedMove_ = Q_NULLPTR;

    return true;
}

QString DeleteModifyCommand::string() const
{
    return getShowString(ComIndex::DeleteMove, curZhStr);
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

bool CommandContainer::append(Command* command)
{
    bool success = command->execute();
    if (success) {
        clearRecovers();
        revokeCommands.push(command);
    } else
        delete command;

    return success;
}

bool CommandContainer::revoke(int num)
{
    bool success { false };
    while (num-- > 0 && !revokeCommands.isEmpty()) {
        Command* command = revokeCommands.pop();
        recoverCommands.push(command);
        success = command->unExecute();
    }

    return success;
}

bool CommandContainer::recover(int num)
{
    bool success { false };
    while (num-- > 0 && !recoverCommands.isEmpty()) {
        Command* command = recoverCommands.pop();
        revokeCommands.push(command);
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
