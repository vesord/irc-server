#include "gtest/gtest.h"

#include "ACommand.hpp"
#include "Parser.hpp"


TEST(test, test_of_test) {
	SUCCEED();
}

/*
TEST(getNameOfCmd, find) {
	static const char *		crlf = "\r\n";

	ASSERT_EQ(getCommandNameByLine(""), "");
	ASSERT_EQ(getCommandNameByLine("COMMAND"), "");
	ASSERT_EQ(getCommandNameByLine("     COMMAND\r\n"), "");

	ASSERT_EQ(getCommandNameByLine("COMMAND\r\n"), "COMMAND");
	ASSERT_EQ(getCommandNameByLine(":prefix COMMAND\r\n"), "COMMAND");
	ASSERT_EQ(getCommandNameByLine(":prefix      COMMAND\r\n"), "COMMAND");
	ASSERT_EQ(getCommandNameByLine(":prefix      COMMAND    arg\r\n"), "COMMAND");
	ASSERT_EQ(getCommandNameByLine(":prefix      123    arg\r\n"), "123");
	ASSERT_EQ(getCommandNameByLine(":prefix      command    arg\r\n"), "command");
}

TEST(getCommand, find) {
	ACommand *		ret;

	ASSERT_EQ(getCommandObjectByName(""), nullptr);
	ASSERT_EQ(getCommandObjectByName("asdasda"), nullptr);
	ASSERT_EQ(getCommandObjectByName("\n"), nullptr);

	ASSERT_TRUE(getCommandObjectByName("pass") != nullptr);
	ASSERT_TRUE(getCommandObjectByName("paSs") != nullptr);
	ASSERT_TRUE(getCommandObjectByName("PASS") != nullptr);
}
*/

TEST(parser, get_command) {
	Parser::receive_container	receiveBuffers;
	receiveBuffers[3] = std::string("PASS password") + Parser::crlf;
	receiveBuffers[4] = std::string("SERVER") + Parser::crlf + std::string("SERVER") + Parser::crlf;

	Parser		parser;
	Parser::commands_container	result = parser.getCommandsContainerFromReceiveMap(receiveBuffers);

	ACommand *	cmd;
	int			i = 0;
	while (!result.empty()) {
		cmd = result.front();
		result.pop();
		++i;
	}
	ASSERT_EQ(i, 3);
}

TEST(parser, cutstring1) {
	ACommand::command_prefix_t pfx;

	std::string s1 = ":name!user@host pass args";
	std::string s2 = ":name pass";
	std::string s3 = ":name!user pass";
	std::string s4 = ":name@host pass";

	Parser::fillPrefix(pfx, s1);
	EXPECT_EQ("name", pfx.name);
	EXPECT_EQ("user", pfx.user);
	EXPECT_EQ("host", pfx.host);
	EXPECT_EQ(":name!user@host", pfx.toString());

	Parser::fillPrefix(pfx, s2);
	EXPECT_EQ("name", pfx.name);
	EXPECT_EQ("", pfx.user);
	EXPECT_EQ("", pfx.host);
	EXPECT_EQ(":name", pfx.toString());

	Parser::fillPrefix(pfx, s3);
	EXPECT_EQ("name", pfx.name);
	EXPECT_EQ("user", pfx.user);
	EXPECT_EQ("", pfx.host);
	EXPECT_EQ(":name!user", pfx.toString());

	Parser::fillPrefix(pfx, s4);
	EXPECT_EQ("name", pfx.name);
	EXPECT_EQ("", pfx.user);
	EXPECT_EQ("host", pfx.host);
	EXPECT_EQ(":name@host", pfx.toString());

	Parser::fillPrefix(pfx, ":name!user@host");
	EXPECT_EQ("name", pfx.name);
	EXPECT_EQ("user", pfx.user);
	EXPECT_EQ("host", pfx.host);
	EXPECT_EQ(":name!user@host", pfx.toString());
}

std::list<std::string> splitArgs(const std::string & strIn) {
    std::string::size_type  pos = 0;
    std::list<std::string>  result;

    const std::string withoutCrLf = strIn.substr(0, strIn.find(Parser::crlf));
    std::string strFirst = withoutCrLf.substr(0, withoutCrLf.find(':', 1));
    std::string strSecond;
    if ((pos = withoutCrLf.find(':', 1)) != std::string::npos) {
        strSecond = withoutCrLf.substr(pos, withoutCrLf.length() - pos);
    }

    pos = 0;
    while (pos != std::string::npos) {
        pos = strFirst.find(Parser::space);
        result.push_back(strFirst.substr(0, pos));
        pos = strFirst.find_first_not_of(Parser::space, pos);
        strFirst.erase(0, pos);
    }
    if (!strSecond.empty()) {
        result.push_back(strSecond);
    }
    return result;
}

TEST(parserSplit, testsplit) {
    std::string             input(":prefix!pr@pr2 Command Arg1 Arg2 :Arg3 Arg4 :Arg5\r\n");
    std::list<std::string>  expect;
    expect.push_back(":prefix!pr@pr2");
    expect.push_back("Command");
    expect.push_back("Arg1");
    expect.push_back("Arg2");
    expect.push_back(":Arg3 Arg4 :Arg5");
    ASSERT_EQ(expect, splitArgs(input));

    std::string             input2(":prefix!pr@pr2 Command Arg1 Arg2 Arg3\r\n");
    std::list<std::string>  expect2;
    expect2.push_back(":prefix!pr@pr2");
    expect2.push_back("Command");
    expect2.push_back("Arg1");
    expect2.push_back("Arg2");
    expect2.push_back("Arg3");
    ASSERT_EQ(expect2, splitArgs(input2));

    std::string             input3("Command Arg1 Arg2 Arg3\r\n");
    std::list<std::string>  expect3;
    expect3.push_back("Command");
    expect3.push_back("Arg1");
    expect3.push_back("Arg2");
    expect3.push_back("Arg3");
    ASSERT_EQ(expect3, splitArgs(input3));

    std::string             input4("\r\n");
    std::list<std::string>  expect4;
    ASSERT_EQ(expect4, splitArgs(input4));
}

