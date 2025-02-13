#ifndef ILOG_H
#define ILOG_H
#include <string>
#include <typeindex>

class IErrorLog
{
public:
	virtual void log(std::string src, std::string err, std::string message) = 0;
};

#endif
