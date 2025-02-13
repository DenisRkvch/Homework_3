#include "CmdRunner.h"
#include "ExceptionHandler.h"
#include <iostream>
#include <exception>
#include "Commands.h"


CmdRunner::CmdRunner()
{
}

CmdRunner::~CmdRunner()
{
}

void CmdRunner::run(void)
{
	_stop = false;

    while (!_stop)
    {
        if (queue.empty()) continue;

        ICommand* cmd = queue.front();
        queue.pop();

        try {

            cmd->execute();

            delete cmd;

        }
        catch (std::exception* e) {

            ICommand* exceptionComand = ExceptionHandler::handle(cmd, e);
            exceptionComand->execute();

            delete exceptionComand;
        }   

        empty = queue.empty();
    }
}

void CmdRunner::addCmd(ICommand* c)
{
    queue.push(c);
}

bool CmdRunner::isEmpty(void)
{
    return empty;
}

void CmdRunner::stop(void)
{
	_stop = true;
}


