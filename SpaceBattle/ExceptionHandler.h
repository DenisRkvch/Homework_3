#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include <map>
#include <string>
#include <functional>
#include <typeinfo>
#include <typeindex>
#include <iostream>
#include "ICommand.h"


class ExceptionHandler
{

public:

    ExceptionHandler() = default;
	~ExceptionHandler() = default;

    static ICommand* handle(ICommand* c, std::exception* e);
    static void Register(const std::type_info& ct, const std::type_info& et, std::function<ICommand*(ICommand*, std::exception*)> callback);

private:

	// Команда-обработчик по умолчанию, если не найденна подходящая в _store
    class defaultHandlerCommand : public ICommand
	{
	public:
        defaultHandlerCommand(ICommand* c, std::exception* e) : _c(c), _e(e) {  }

		// Унаследовано через ICommand
		void execute(void) override
		{
            std::cout << "A corresponding handler for Command: " << typeid(*_c).name() <<
                ", Exception class: " << typeid(*_e).name() << " is not declared! \n";
            delete _c;
		}
	private:
		ICommand* _c;
		std::exception* _e;
	};

	static std::map<
		std::type_index,
		std::map<
			std::type_index, 
            std::function<ICommand*(ICommand*, std::exception*)>
		>
	>_store;

};

#endif
