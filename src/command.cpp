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
    bool can = board_->canPut(piece_, coord_);
    if (can)
        board_->placePiece(piece_, coord_);

    return can;
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
    piece_ = board_->getPiece(coord_);
    if (piece_)
        board_->takeOutPiece(coord_);

    return piece_;
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
    , oldCurMove_ { manualMove_->move() }
{
}

ModifyCommand::~ModifyCommand()
{
    if (discardMove_)
        Move::deleteMove(discardMove_);
}

AppendModifyCommand::AppendModifyCommand(Manual* manual, const CoordPair& coordPair, bool isOther)
    : ModifyCommand(manual)
{
    coordPair_ = coordPair;
    isOther_ = isOther;
}

bool AppendModifyCommand::execute()
{
    bool isCurMove = manualMove_->isCurMove(oldCurMove_);
    if (isCurMove) {
        if (!isOther_)
            discardMove_ = oldCurMove_->nextMove();

        addMove_ = manualMove_->append_coordPair(coordPair_, "", isOther_);
        if (addMove_) {
            curZhStr = manualMove_->curZhStr();
            if (isOther_)
                addMove_->setOtherMove(oldCurMove_->otherMove());
        }
    }

    return isCurMove && addMove_;
}

bool AppendModifyCommand::unExecute()
{
    bool isCurMove = manualMove_->isCurMove(addMove_);
    if (isCurMove) {
        if (manualMove_->backOther()) {
            manualMove_->move()->setOtherMove(addMove_->otherMove());
            addMove_->setOtherMove(Q_NULLPTR);
        } else if (manualMove_->backNext()) {
            manualMove_->move()->setNextMove(discardMove_);
            discardMove_ = Q_NULLPTR;
        }

        Move::deleteMove(addMove_);
    }

    return isCurMove;
}

QString AppendModifyCommand::string() const
{
    return getShowString(ComIndex::AppendMove, curZhStr);
}

bool DeleteModifyCommand::execute()
{
    bool isCurMove = manualMove_->isCurMove(oldCurMove_);
    if (isCurMove) {
        isOther_ = oldCurMove_->isOther();
        if (manualMove_->backOther()) {
            manualMove_->move()->setOtherMove(oldCurMove_->otherMove());
            oldCurMove_->setOtherMove(Q_NULLPTR);
        } else if (manualMove_->backNext())
            manualMove_->move()->setNextMove(Q_NULLPTR);

        discardMove_ = oldCurMove_;
    }

    return isCurMove;
}

bool DeleteModifyCommand::unExecute()
{
    bool isCurMove = manualMove_->isCurMove(discardMove_->preMove());
    if (isCurMove) {
        if (isOther_) {
            discardMove_->setOtherMove(manualMove_->move()->otherMove());
            manualMove_->move()->setOtherMove(discardMove_);
            discardMove_ = Q_NULLPTR;
        } else {
            Move::deleteMove(manualMove_->move()->nextMove());
            manualMove_->move()->setNextMove(discardMove_);
        }
    }

    return isCurMove;
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
