#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include <QList>

class Move;
class ManualMove;

class MoveCommand {
public:
    MoveCommand(ManualMove*& movCursor);
    virtual ~MoveCommand() = default;

    ManualMove*& moveCursor() const { return moveCursor_; }

    virtual bool execute() = 0;
    virtual bool unExecute() = 0;

    virtual QString exeString() const = 0;
    virtual QString unExeString() const = 0;

private:
    ManualMove*& moveCursor_;
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
    GoToMoveCommand(ManualMove*& movCursor, Move* toMove);

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
    GoIsMoveCommand(ManualMove*& movCursor, bool isOther);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    bool isOther_;
};

class GoIncMoveCommand : public MoveCommand {
public:
    GoIncMoveCommand(ManualMove*& movCursor, int count);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    int count_;
};

class BackIncMoveCommand : public MoveCommand {
public:
    BackIncMoveCommand(ManualMove*& movCursor, int count);

    virtual bool execute();
    virtual bool unExecute();

    virtual QString exeString() const;
    virtual QString unExeString() const;

private:
    int count_;
};

#endif // MOVECOMMAND_H
