#ifndef COMMANDS_H
#define COMMANDS_H

#include "ICommand.h"
#include "CmdRunner.h"
#include "IErrorLog.h"

// Команда, добавляющая другую команду в очередь команд
class AddCommand : public ICommand
{
public:
    AddCommand(CmdRunner* runner, ICommand* command);
    ~AddCommand() override;
    void execute() override;

private:
    CmdRunner* run;
    ICommand* com;

};


// Команда которая записывает информацию о выброшенном исключении в лог
class WriteToLogCommand :public ICommand
{
public:
    WriteToLogCommand(IErrorLog* logger, ICommand* command, std::exception* exception);
    ~WriteToLogCommand() override;
    void execute(void) override;

private:
    ICommand* com;
    std::exception* exc;
    IErrorLog* log;
};

// Команда, которая повторяет Команду, выбросившую исключение
class RepeatCommand :public ICommand
{
public:
    RepeatCommand(ICommand *command);
    ~RepeatCommand() override;
    void execute(void) override;

private:
    ICommand* com;
};

// Команда, которая повторяет Команду, выбросившую исключение
class RepeatCommand2nd :public ICommand
{
public:
    RepeatCommand2nd(ICommand *command);
    ~RepeatCommand2nd() override;
    void execute(void) override;

private:
    ICommand* com;
};

#endif // COMMANDS_H
