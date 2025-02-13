#include "ExceptionHandler.h"

ICommand* ExceptionHandler::handle(ICommand* c, std::exception* e)
{
    if (_store.find(typeid(*c)) == _store.end()) {
        // если по первому ключу не найдено, то вернуть исключение по умолчанию
        return new defaultHandlerCommand(c, e);
    }else if (_store[typeid(*c)].find(typeid(*e)) == _store[typeid(*c)].end()) {
        // если по второму ключу не найдено, то вернуть исключение по умолчанию
        return new defaultHandlerCommand(c, e);
    }

    return _store[typeid(*c)][typeid(*e)](c, e);
}

void ExceptionHandler::Register(const std::type_info& ct, const std::type_info& et, std::function<ICommand*(ICommand*, std::exception*)> callback)
{

	_store[std::type_index(ct)][std::type_index(et)] = callback;

}

// define static veriable 
std::map<std::type_index, std::map<std::type_index, std::function<ICommand*(ICommand*, std::exception*)>>> ExceptionHandler::_store;

