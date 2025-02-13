#ifndef CMDRUNNER_H
#define CMDRUNNER_H

#include <queue>
#include "ICommand.h"

class CmdRunner
{
public:

	CmdRunner();
	~CmdRunner();

	void run(void);
	void addCmd(ICommand* c);
	bool isEmpty(void);
	void stop(void);

private:

	std::queue<ICommand*> queue;

	bool _stop = 0;
	bool empty = 0;
	
};
#endif

