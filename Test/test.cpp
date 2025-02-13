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
	~SomeCommandMock() override { Die(); }	// ��� �������� ������ �����������
	MOCK_METHOD((void), Die, ());
};

class SomeExceptionMock : public std::exception {
public:
	MOCK_METHOD((const char*), what, (), (const noexcept override));
	~SomeExceptionMock() override { Die(); }	// ��� �������� ������ �����������
	MOCK_METHOD((void), Die, ());

};

// ----- ���� ������� ������� ���������� ���������� � ����������� ���������� � ���. -----
TEST(DZ_Punkt_4, WriteToLogCommand_execute)
{
	CmdRunner runner;
	ErrorLogMock logger;
	SomeCommandMock* error_command = new SomeCommandMock();
	SomeExceptionMock* error = new SomeExceptionMock();

	WriteToLogCommand* writeToLogCommand_test = new WriteToLogCommand(&logger, error_command, error);

	EXPECT_CALL(*error, what()).WillOnce(Return("test"));
	EXPECT_CALL(logger, log(typeid(SomeCommandMock).name(), typeid(SomeExceptionMock).name(), "test")).Times(1);

	//�������� ������������ ������ ����������� ��������� ��������
	EXPECT_CALL(*error_command, Die());
	EXPECT_CALL(*error, Die());

	runner.addCmd(writeToLogCommand_test);

	// ������ ������� � �������� ������
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// �������� �������, ���� �������� ��� �������
	while (!runner.isEmpty()) { ; };
	runner.stop();


}

// ----- ���� ����������� ����������, ������� ������ �������, ������� � ��� � ������� ������: ------
// 1. ���� ��������, ����������� � ������� ������ �������
TEST(DZ_Punkt_5, AddCommand)
{
	CmdRunner runner;
	SomeCommandMock* target_command = new SomeCommandMock();

	AddCommand* addCommand_test = new AddCommand(&runner, target_command);

	EXPECT_CALL(*target_command, execute()).Times(1);
	
	//�������� ������������ ������ ����������� ��������� ��������
	EXPECT_CALL(*target_command, Die());


	runner.addCmd(addCommand_test);

	// ������ ������� � �������� ������
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// �������� �������, ���� �������� ��� �������
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// 2. ���� ����������� ����������, ������� ��������� �������:
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

	//�������� ������������ ������ ����������� ��������� ��������
	EXPECT_CALL(*start_command, Die());
	EXPECT_CALL(*test_command, Die());
	EXPECT_CALL(*error, Die());


	runner.addCmd(start_command);

	// ������ ������� � �������� ������
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// �������� �������, ���� �������� ��� �������
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// 3. ���� ����������� ����������, ������� ������ �������, ������� � ��� � ������� ������:
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

	//�������� ������������ ������ ����������� ��������� ��������
	EXPECT_CALL(*error_command, Die());

	runner.addCmd(error_command);

	// ������ ������� � �������� ������
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// �������� �������, ���� �������� ��� �������
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// ���� �������, ������� ��������� ������� .
TEST(DZ_Punkt_6, RepeatCommand_execute)
{
	CmdRunner runner;
	SomeCommandMock* target_command = new SomeCommandMock();
	RepeatCommand* repeatCommand_test = new RepeatCommand(target_command);

	EXPECT_CALL(*target_command, execute()).Times(1);

	//�������� ������������ ������ ����������� ��������� ��������
	EXPECT_CALL(*target_command, Die());

	runner.addCmd(repeatCommand_test);

	// ������ ������� � �������� ������
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// �������� �������, ���� �������� ��� �������
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// ���� ����������� ����������, ������� ������ � ������� ������� - ����������� �������, ����������� ����������.
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

	//�������� ������������ ������ ����������� ��������� ��������
	EXPECT_CALL(*error_command, Die());
	EXPECT_CALL(*error, Die());

	runner.addCmd(error_command);

	// ������ ������� � �������� ������
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// �������� �������, ���� �������� ��� �������
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// ���� ���������: 
//		� ������� ������ �� ������ 4 � ������ 6 ����������� ��������� ��������� ����������:
//		��� ������ ������� ���������� ��������� �������, ��� ��������� ������� ���������� �������� ���������� � ���.

TEST(DZ_Punkt_8, Exception_Handler_RepeatCommand_Than_WriteToLog)
{
	CmdRunner runner;
	ErrorLogMock logger;
	SomeCommandMock* error_command = new SomeCommandMock();
	SomeExceptionMock* error_fst = new SomeExceptionMock();
	SomeExceptionMock* error_snd = new SomeExceptionMock();

	// ����������  ����������, ������� ������ � ������� ������� - ����������� �������, ����������� ����������
	auto exception_handler_repeat = [&runner](ICommand* c, std::exception* e) -> ICommand* {
		delete e;
		return new AddCommand(&runner, new RepeatCommand(c));
		};

	// ���������� ����������, ������� ������ �������, ������� � ��� � ������� ������
	auto exception_handle_WriteToLog = [&runner, &logger](ICommand* c, std::exception* e) -> ICommand* {
		return new AddCommand(&runner, new WriteToLogCommand(&logger, c, e));
		};

	ExceptionHandler::Register(typeid(SomeCommandMock), typeid(SomeExceptionMock), exception_handler_repeat);
	ExceptionHandler::Register(typeid(RepeatCommand), typeid(SomeExceptionMock), exception_handle_WriteToLog);

	EXPECT_CALL(*error_command, execute()).Times(2).WillOnce(Throw(error_fst)).WillOnce(Throw(error_snd));
	EXPECT_CALL(*error_snd, what()).WillOnce(Return("test"));
	EXPECT_CALL(logger, log(typeid(RepeatCommand).name(), typeid(SomeExceptionMock).name(), "test")).Times(1);

	//�������� ������������ ������ ����������� ��������� ��������
	EXPECT_CALL(*error_command, Die());
	EXPECT_CALL(*error_fst, Die());
	EXPECT_CALL(*error_snd, Die());

	runner.addCmd(error_command);

	// ������ ������� � �������� ������
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// �������� �������, ���� �������� ��� �������
	while (!runner.isEmpty()) { ; };
	runner.stop();
}

// ���� ���������:
//		����������� ��������� ��������� ���������� - ��������� ��� ����, ����� �������� � ���.
//		��������: ������� ����� �������, ����� ����� �� ��� � ������ 6. 
//		��� ���� ������� ����� ����������, ��� ������� �� ������� ��������� ��� ����.
TEST(DZ_Punkt_9, Exception_Handler_Repeat_Repeat_WriteToLog)
{
	CmdRunner runner;
	ErrorLogMock logger;
	SomeCommandMock* error_command = new SomeCommandMock();
	SomeExceptionMock* error_fst = new SomeExceptionMock();
	SomeExceptionMock* error_snd = new SomeExceptionMock();
	SomeExceptionMock* error_trd = new SomeExceptionMock();

	// ����������  ����������, ������� ������ � ������� ������� - ����������� �������, ����������� ����������
	auto exhdl_RepeatTwice = [&runner](ICommand* c, std::exception* e) -> ICommand* {
		delete e;
		return new AddCommand(&runner, new RepeatCommand2nd(c));
	};

	// ����������  ����������, ������� ������ � ������� ������� - ����������� �������, ����������� ���������� �� ����� �������
	auto exhdl_Repeat = [&runner](ICommand* c, std::exception* e) -> ICommand* {
		delete e;
		return new AddCommand(&runner, new RepeatCommand(c));
	};

	// ���������� ����������, ������� ������ �������, ������� � ��� � ������� ������
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

	//�������� ������������ ������ ����������� ��������� ��������
	EXPECT_CALL(*error_command, Die());
	EXPECT_CALL(*error_fst, Die());
	EXPECT_CALL(*error_snd, Die());
	EXPECT_CALL(*error_trd, Die());

	runner.addCmd(error_command);

	// ������ ������� � �������� ������
	std::thread thr(&CmdRunner::run, &runner);
	thr.detach();

	// �������� �������, ���� �������� ��� �������
	while (!runner.isEmpty()) { ; };
	runner.stop();
}
