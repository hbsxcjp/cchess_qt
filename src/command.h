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

class Command {
public:
    virtual ~Command() = default;

    virtual bool execute() = 0;
    virtual bool unExecute() = 0;

    virtual QString string() const = 0;
};

class PutCommand : public Command {
public:
    PutCommand(Manual* manual, const Coord& coord);

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

    virtual QString string() const;
};

class TakeOutPutCommand : public PutCommand {
public:
    using PutCommand::PutCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class MoveCommand : public Command {
public:
    MoveCommand(Manual* manual);

protected:
    ManualMove* manualMove_;

    Move* fromMove_;
    Move* toMove_ {};
    QString curZhStr {};
};

class GoNextMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class BackNextMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class GoOtherMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class BackOtherMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class BackToPreMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class GoEndMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class BackStartMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class GoToMoveCommand : public MoveCommand {
public:
    GoToMoveCommand(Manual* manual, Move* toMove);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class GoIncMoveCommand : public MoveCommand {
public:
    GoIncMoveCommand(Manual* manual, int count);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;

private:
    int count_;
};

class BackIncMoveCommand : public MoveCommand {
public:
    BackIncMoveCommand(Manual* manual, int count);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;

private:
    int count_;
};

class ModifyCommand : public Command {
public:
    ModifyCommand(Manual* manual);
    virtual ~ModifyCommand();

protected:
    ManualMove* manualMove_;
    Move* oldCurMove_ {};

    CoordPair coordPair_ {};
    bool isOther_ {};
    Move* addMove_ {};
    Move* discardMove_ {};
    QString curZhStr {};
};

class AppendModifyCommand : public ModifyCommand {
public:
    AppendModifyCommand(Manual* manual, const CoordPair& coordPair, bool isOther);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class DeleteModifyCommand : public ModifyCommand {
public:
    using ModifyCommand::ModifyCommand;
    virtual ~DeleteModifyCommand();

    virtual bool execute();
    virtual bool unExecute();

    virtual QString string() const;
};

class CommandContainer {
public:
    CommandContainer();
    ~CommandContainer();

    bool append(Command* command);
    bool revoke(int num);
    bool recover(int num);

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
