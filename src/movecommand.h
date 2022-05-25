#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include <QList>
#include <QStack>

class Move;
class ManualMove;

class MoveCommand {
public:
    MoveCommand(ManualMove* manualMove);
    virtual ~MoveCommand() = default;

    virtual bool execute() = 0;
    virtual bool unExecute() = 0;

    virtual QString exeString() const = 0;
    virtual QString unExeString() const = 0;

protected:
    ManualMove* manualMove_;
};

class GoNextMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;
};

class BackNextMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;
};

class GoOtherMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;
};

class BackOtherMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;
};

class BackToPreMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    Move* fromMove_ {};
};

class GoEndMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    Move* endMove_ {};
};

class BackStartMoveCommand : public MoveCommand {
public:
    using MoveCommand::MoveCommand;

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    Move* endMove_ {};
};

class GoToMoveCommand : public MoveCommand {
public:
    GoToMoveCommand(ManualMove* movCursor, Move* toMove);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    Move* fromMove_ {};
    Move* toMove_ {};
};

class GoIsMoveCommand : public MoveCommand {
public:
    GoIsMoveCommand(ManualMove* movCursor, bool isOther);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    bool isOther_;
};

class GoIncMoveCommand : public MoveCommand {
public:
    GoIncMoveCommand(ManualMove* movCursor, int count);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    int count_;
};

class BackIncMoveCommand : public MoveCommand {
public:
    BackIncMoveCommand(ManualMove* movCursor, int count);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    int count_;
};

class MoveCommandContainer {
public:
    MoveCommandContainer();
    ~MoveCommandContainer();

    void append(MoveCommand* command);

    void revoke();
    void recover();

private:
    MoveCommand* moveTop(QStack<MoveCommand*>& fromCommands, QStack<MoveCommand*>& toCommands);

    QStack<MoveCommand*> revokeCommands;
    QStack<MoveCommand*> recoverCommands;
};

#endif // MOVECOMMAND_H
