#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include <QList>
#include <QStack>

class Move;
class Manual;
class ManualMove;

class MoveCommand {
public:
    MoveCommand(Manual* manual);
    virtual ~MoveCommand() = default;

    virtual bool execute() = 0;
    virtual bool unExecute() = 0;

    virtual QString string() const = 0;

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

class MoveCommandContainer {
public:
    MoveCommandContainer();
    ~MoveCommandContainer();

    bool append(MoveCommand* command);

    QStringList getRevokeStrings() const;
    QStringList getRecoverStrings() const;

    bool revoke(int num);
    bool recover(int num);

    void clearRevokes();
    void clearRecovers();

private:
    QStack<MoveCommand*> revokeCommands;
    QStack<MoveCommand*> recoverCommands;
};

#endif // MOVECOMMAND_H
