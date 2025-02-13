#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <thread>

#include "../SpaceBattle/CmdRunner.h"
#include "../SpaceBattle/Commands.h"
#include "../SpaceBattle/IErrorLog.h"
#include "../SpaceBattle/ICommand.h"
#include "../SpaceBattle/ExceptionHandler.h"


using ::testing::Return;
using ::testing::Throw;


class ErrorLogMock : public IErrorLog {
public:
	MOCK_METHOD((void), log, (std::string, std::string, std::string), (override));
};

class SomeCommandMock : public ICommand {
public:
	MOCK_METHOD((void), execute, (), (override));
	~SomeCommandMock() override { Die(); }	// для проверки вызова деструктора
	MOCK_METHOD((void), Die, ());
};

class SomeExceptionMock : public std::exception {
public:
	MOCK_METHOD((const char*), what, (), (const noexcept override));
	~SomeExceptionMock() override { Die(); }	// для проверки вызова деструктора
	MOCK_METHOD((void), Die, ());

};

// ----- тест Команды которая записывает информацию о выброшенном исключении в лог. -----
TEST(DZ_Punkt_4, WriteToLogCommand_execute)
{
	CmdRunner runner;
	ErrorLogMock logger;
	SomeCommandMock* error_command = new SomeCommandMock();
	SomeExceptionMock* error = new SomeExceptionMock();

	WriteToLogCommand* writeToLogCommand_test = new WriteToLogCommand(&logger, error_command, error);

	EXPECT_CALL(*error, what()).WillOnce(Return("test"));
	EXPECT_CALL(logger, log(typeid(SomeCommandMock).name(), typeid(SomeExceptionMock).name(), "test")).Times(1);

	//Проверка освобождения памяти динамически созданных объектов
	EXPECT_CALL(*error_command, Die());
	EXPECT_CALL(*error, Die());

	runner.addCmd(writeToLogCommand_test);

	// запуск раннера в отдльном потоке
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// ожидание раннера, пока выполнит все команды
	while (!runner.isEmpty()) { ; };
	runner.stop();


}

// ----- тест обработчика исключения, который ставит Команду, пишущую в лог в очередь Команд: ------
// 1. Тест комманды, добавляющей в очередь другую команду
TEST(DZ_Punkt_5, AddCommand)
{
	CmdRunner runner;
	SomeCommandMock* target_command = new SomeCommandMock();

	AddCommand* addCommand_test = new AddCommand(&runner, target_command);

	EXPECT_CALL(*target_command, execute()).Times(1);
	
	//Проверка освобождения памяти динамически созданных объектов
	EXPECT_CALL(*target_command, Die());


	runner.addCmd(addCommand_test);

	// запуск раннера в отдльном потоке
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// ожидание раннера, пока выполнит все команды
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// 2. Тест обработчика исключения, который выполняет команду:
TEST(DZ_Punkt_5, Exception_Handler)
{
	CmdRunner runner;
	SomeCommandMock* start_command = new SomeCommandMock();
	SomeCommandMock* test_command = new SomeCommandMock();
	SomeExceptionMock* error = new SomeExceptionMock();

	auto exception_handle = [&test_command](ICommand* c, std::exception* e) -> ICommand* {
		delete e;
		delete c;
		return test_command;
		};

	ExceptionHandler::Register(typeid(SomeCommandMock), typeid(SomeExceptionMock), exception_handle);

	EXPECT_CALL(*start_command, execute()).WillOnce(Throw(error));
	EXPECT_CALL(*test_command, execute()).Times(1);

	//Проверка освобождения памяти динамически созданных объектов
	EXPECT_CALL(*start_command, Die());
	EXPECT_CALL(*test_command, Die());
	EXPECT_CALL(*error, Die());


	runner.addCmd(start_command);

	// запуск раннера в отдльном потоке
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// ожидание раннера, пока выполнит все команды
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// 3. тест обработчика исключения, который ставит Команду, пишущую в лог в очередь Команд:
TEST(DZ_Punkt_5, Exception_Handler_AddWriteToLogCommand)
{
	CmdRunner runner;
	ErrorLogMock logger;
	SomeCommandMock* error_command = new SomeCommandMock();

	auto exception_handle_WriteToLog = [&runner, &logger](ICommand* c, std::exception* e) -> ICommand* {
		return new AddCommand(&runner, new WriteToLogCommand(&logger, c, e));
		};

	ExceptionHandler::Register(typeid(SomeCommandMock), typeid(std::exception), exception_handle_WriteToLog);

	EXPECT_CALL(*error_command, execute()).WillOnce(Throw(new std::exception()));
	EXPECT_CALL(logger, log(typeid(SomeCommandMock).name(), typeid(std::exception).name(), testing::_)).Times(1);

	//Проверка освобождения памяти динамически созданных объектов
	EXPECT_CALL(*error_command, Die());

	runner.addCmd(error_command);

	// запуск раннера в отдльном потоке
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// ожидание раннера, пока выполнит все команды
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// Тест команды, которая повторяет команду .
TEST(DZ_Punkt_6, RepeatCommand_execute)
{
	CmdRunner runner;
	SomeCommandMock* target_command = new SomeCommandMock();
	RepeatCommand* repeatCommand_test = new RepeatCommand(target_command);

	EXPECT_CALL(*target_command, execute()).Times(1);

	//Проверка освобождения памяти динамически созданных объектов
	EXPECT_CALL(*target_command, Die());

	runner.addCmd(repeatCommand_test);

	// запуск раннера в отдльном потоке
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// ожидание раннера, пока выполнит все команды
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// Тест обработчика исключения, который ставит в очередь Команду - повторитель команды, выбросившей исключение.
TEST(DZ_Punkt_7, Exception_Handler_RepeatCommand)
{
	CmdRunner runner;
	SomeCommandMock* error_command = new SomeCommandMock();
	SomeExceptionMock* error = new SomeExceptionMock();

	auto exception_handler_repeat = [&runner](ICommand* c, std::exception* e) -> ICommand* {
		delete e;
		return new AddCommand(&runner, new RepeatCommand(c));
		};

	ExceptionHandler::Register(typeid(SomeCommandMock), typeid(SomeExceptionMock), exception_handler_repeat);

	EXPECT_CALL(*error_command, execute()).Times(2).WillOnce(Throw(error)).WillOnce(Return());

	//Проверка освобождения памяти динамически созданных объектов
	EXPECT_CALL(*error_command, Die());
	EXPECT_CALL(*error, Die());

	runner.addCmd(error_command);

	// запуск раннера в отдльном потоке
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// ожидание раннера, пока выполнит все команды
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// Тест стратегии: 
//		С помощью Команд из пункта 4 и пункта 6 реализовать следующую обработку исключений:
//		при первом выбросе исключения повторить команду, при повторном выбросе исключения записать информацию в лог.

TEST(DZ_Punkt_8, Exception_Handler_RepeatCommand_Than_WriteToLog)
{
	CmdRunner runner;
	ErrorLogMock logger;
	SomeCommandMock* error_command = new SomeCommandMock();
	SomeExceptionMock* error_fst = new SomeExceptionMock();
	SomeExceptionMock* error_snd = new SomeExceptionMock();

	// обработчик  исключения, который ставит в очередь Команду - повторитель команды, выбросившей исключение
	auto exception_handler_repeat = [&runner](ICommand* c, std::exception* e) -> ICommand* {
		delete e;
		return new AddCommand(&runner, new RepeatCommand(c));
		};

	// обработчик исключения, который ставит Команду, пишущую в лог в очередь команд
	auto exception_handle_WriteToLog = [&runner, &logger](ICommand* c, std::exception* e) -> ICommand* {
		return new AddCommand(&runner, new WriteToLogCommand(&logger, c, e));
		};

	ExceptionHandler::Register(typeid(SomeCommandMock), typeid(SomeExceptionMock), exception_handler_repeat);
	ExceptionHandler::Register(typeid(RepeatCommand), typeid(SomeExceptionMock), exception_handle_WriteToLog);

	EXPECT_CALL(*error_command, execute()).Times(2).WillOnce(Throw(error_fst)).WillOnce(Throw(error_snd));
	EXPECT_CALL(*error_snd, what()).WillOnce(Return("test"));
	EXPECT_CALL(logger, log(typeid(RepeatCommand).name(), typeid(SomeExceptionMock).name(), "test")).Times(1);

	//Проверка освобождения памяти динамически созданных объектов
	EXPECT_CALL(*error_command, Die());
	EXPECT_CALL(*error_fst, Die());
	EXPECT_CALL(*error_snd, Die());

	runner.addCmd(error_command);

	// запуск раннера в отдльном потоке
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// ожидание раннера, пока выполнит все команды
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// Тест стратегии:
//		Реализовать стратегию обработки исключения - повторить два раза, потом записать в лог.
//		Указание: создать новую команду, точно такую же как в пункте 6. 
//		Тип этой команды будет показывать, что Команду не удалось выполнить два раза.
TEST(DZ_Punkt_9, Exception_Handler_Repeat_Repeat_WriteToLog)
{
	CmdRunner runner;
	ErrorLogMock logger;
	SomeCommandMock* error_command = new SomeCommandMock();
	SomeExceptionMock* error_fst = new SomeExceptionMock();
	SomeExceptionMock* error_snd = new SomeExceptionMock();
	SomeExceptionMock* error_trd = new SomeExceptionMock();

	// обработчик  исключения, который ставит в очередь Команду - повторитель команды, выбросившей исключение
	auto exhdl_RepeatTwice = [&runner](ICommand* c, std::exception* e) -> ICommand* {
		delete e;
		return new AddCommand(&runner, new RepeatCommand2nd(c));
	};

	// обработчик  исключения, который ставит в очередь Команду - повторитель команды, выбросившей исключение во время повтора
	auto exhdl_Repeat = [&runner](ICommand* c, std::exception* e) -> ICommand* {
		delete e;
		return new AddCommand(&runner, new RepeatCommand(c));
	};

	// обработчик исключения, который ставит Команду, пишущую в лог в очередь команд
	auto exhdl_WriteToLog = [&runner, &logger](ICommand* c, std::exception* e) -> ICommand* {
		return new AddCommand(&runner, new WriteToLogCommand(&logger, c, e));
	};

	ExceptionHandler::Register(typeid(SomeCommandMock), typeid(SomeExceptionMock), exhdl_RepeatTwice);
	ExceptionHandler::Register(typeid(RepeatCommand2nd), typeid(SomeExceptionMock), exhdl_Repeat);
	ExceptionHandler::Register(typeid(RepeatCommand), typeid(SomeExceptionMock), exhdl_WriteToLog);

	EXPECT_CALL(*error_command, execute())
		.Times(3)
		.WillOnce(Throw(error_fst))
		.WillOnce(Throw(error_snd))
		.WillOnce(Throw(error_trd));
	EXPECT_CALL(*error_trd, what()).WillOnce(Return("test"));
	EXPECT_CALL(logger, log(typeid(RepeatCommand).name(), typeid(SomeExceptionMock).name(), "test")).Times(1);

	//Проверка освобождения памяти динамически созданных объектов
	EXPECT_CALL(*error_command, Die());
	EXPECT_CALL(*error_fst, Die());
	EXPECT_CALL(*error_snd, Die());
	EXPECT_CALL(*error_trd, Die());

	runner.addCmd(error_command);

	// запуск раннера в отдльном потоке
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// ожидание раннера, пока выполнит все команды
	while (!runner.isEmpty()) { ; };
	runner.stop();
}
