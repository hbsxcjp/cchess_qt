#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include <QList>
#include <QStack>

class Piece;
class Seat;
using Coord = QPair<int, int>;
using CoordPair = QPair<Coord, Coord>;
class Board;

class Move;
class Manual;
class ManualMove;

enum class CommandType {
    Put,
    MoveModify,
    MoveWalk
};

enum ComIndex {
    // 放棋
    PlacePiece,
    TakeOutPiece,

    // 修改
    AppendMove,
    DeleteMove,

    // 移动
    GoNext,
    BackNext,
    GoOther,
    BackOther,
    BackToPre,
    GoEnd,
    BackStart,
    GoTo,
    GoInc,
    BackInc,
};

class Command {
public:
    virtual ~Command() = default;

    virtual CommandType type() const = 0;

    virtual bool execute() = 0;
    virtual bool unExecute() = 0;

    QString string() const;

protected:
    int index;
    QString caption_;
};

class PutCommand : public Command {
public:
    PutCommand(Manual* manual, const Coord& coord);

    virtual CommandType type() const { return CommandType::Put; }

protected:
    Coord coord_;
    Piece* piece_;

    Board* board_;
};

class PlacePutCommand : public PutCommand {
public:
    PlacePutCommand(Manual* manual, Piece* piece, const Coord& coord);

    virtual bool execute();
    virtual bool unExecute();
};

class TakeOutPutCommand : public PutCommand {
public:
    TakeOutPutCommand(Manual* manual, const Coord& coord);

    virtual bool execute();
    virtual bool unExecute();
};

class MoveCommand : public Command {
public:
    MoveCommand(Manual* manual);

    virtual bool execute();
    virtual bool unExecute();

protected:
    virtual bool coreExecute() = 0;
    virtual bool unCoreExecute() = 0;

    ManualMove* manualMove_;

    Move* curMove_;
    Move* doneCurMove_ {};
};

class MoveModifyCommand : public MoveCommand {
public:
    MoveModifyCommand(Manual* manual);
    virtual ~MoveModifyCommand();

    virtual CommandType type() const { return CommandType::MoveModify; }

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();

    virtual bool doCoreExecute() = 0;
    virtual bool unDoCoreExecute() = 0;

    bool markDeleteMove();
    bool removeMarkDeleteMove();

    Move* markDeletedMove_ {};
    CoordPair coordPair_ {};
    bool isOther_ {};
};

class AppendModifyCommand : public MoveModifyCommand {
public:
    AppendModifyCommand(Manual* manual, const CoordPair& coordPair);

protected:
    virtual bool doCoreExecute();
    virtual bool unDoCoreExecute();
};

class DeleteModifyCommand : public MoveModifyCommand {
public:
    DeleteModifyCommand(Manual* manual);

protected:
    virtual bool doCoreExecute();
    virtual bool unDoCoreExecute();
};

class MoveWalkCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual CommandType type() const { return CommandType::MoveWalk; }
};

class GoNextMoveCommand : public MoveWalkCommand {
public:
    GoNextMoveCommand(Manual* manual);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();
};

class BackNextMoveCommand : public MoveWalkCommand {
public:
    BackNextMoveCommand(Manual* manual);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();
};

class GoOtherMoveCommand : public MoveWalkCommand {
public:
    GoOtherMoveCommand(Manual* manual);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();
};

class BackOtherMoveCommand : public MoveWalkCommand {
public:
    BackOtherMoveCommand(Manual* manual);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();
};

class BackToPreMoveCommand : public MoveWalkCommand {
public:
    BackToPreMoveCommand(Manual* manual);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();
};

class GoEndMoveCommand : public MoveWalkCommand {
public:
    GoEndMoveCommand(Manual* manual);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();
};

class BackStartMoveCommand : public MoveWalkCommand {
public:
    BackStartMoveCommand(Manual* manual);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();
};

class GoToMoveCommand : public MoveWalkCommand {
public:
    GoToMoveCommand(Manual* manual, Move* toMove);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();
};

class GoIncMoveCommand : public MoveWalkCommand {
public:
    GoIncMoveCommand(Manual* manual, int count);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();

private:
    int count_;
};

class BackIncMoveCommand : public MoveWalkCommand {
public:
    BackIncMoveCommand(Manual* manual, int count);

protected:
    virtual bool coreExecute();
    virtual bool unCoreExecute();

private:
    int count_;
};

class CommandContainer {
public:
    CommandContainer();
    ~CommandContainer();

    bool append(Command* command, bool allowPush);
    bool revoke(int num, CommandType& type);
    bool recover(int num, CommandType& type);

    void clear();
    void clearRevokes();
    void clearRecovers();

    QStringList getRevokeStrings() const;
    QStringList getRecoverStrings() const;

private:
    QStack<Command*> revokeCommands;
    QStack<Command*> recoverCommands;
};

#endif // MOVECOMMAND_H
