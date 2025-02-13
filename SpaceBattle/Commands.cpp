#include "Commands.h"
#include <iostream>

// комманда, добавляющая другую команду в очередь команд
AddCommand::AddCommand(CmdRunner *runner, ICommand *command) : run(runner), com(command)
{
}

AddCommand::~AddCommand()
{
}

void AddCommand::execute()
{
    run->addCmd(com);
}


// Команда которая записывает информацию о выброшенном исключении в лог
WriteToLogCommand::WriteToLogCommand(IErrorLog* logger, ICommand* command, std::exception* exception) : log(logger), com(command), exc(exception)
{
}

WriteToLogCommand::~WriteToLogCommand()
{
    delete com;
    delete exc;
}

void WriteToLogCommand::execute()
{
    log->log(std::string(typeid(*com).name()), std::string(typeid(*exc).name()), std::string(exc->what()));
}


// Команда, которая повторяет Команду, выбросившую исключение
RepeatCommand::RepeatCommand(ICommand *command) : com(command)
{
}

RepeatCommand::~RepeatCommand()
{
    delete com;
}

void RepeatCommand::execute()
{
    com->execute();
}

// Команда, которая повторяет Команду, выбросившую исключение во второй раз
RepeatCommand2nd::RepeatCommand2nd(ICommand *command) : com(command)
{
}

RepeatCommand2nd::~RepeatCommand2nd()
{
    delete com;
}

void RepeatCommand2nd::execute()
{
    com->execute();
}
