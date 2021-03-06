#include "gtest/gtest.h"

#include "ACommand.hpp"
#include "Parser.hpp"
#include "Modes.hpp"

TEST(parser, get_command) {
	Parser::receive_container	receiveBuffers;
	receiveBuffers[3] = std::string("PASS password") + Parser::crlf;
	receiveBuffers[4] = std::string("PASS") + Parser::crlf + std::string("PASS") + Parser::crlf;

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

TEST(parserSplit, testsplit) {
	std::string				input(":prefix!pr@pr2 Command Arg1 Arg2 :Arg3 Arg4 :Arg5\r\n");
	Parser::arguments_array	expect;
	expect.push_back(":prefix!pr@pr2");
	expect.push_back("Command");
	expect.push_back("Arg1");
	expect.push_back("Arg2");
	expect.push_back(":Arg3 Arg4 :Arg5");
	ASSERT_EQ(expect, Parser::splitArgs(input));

	std::string				input2(":prefix!pr@pr2 Command Arg1 Arg2 Arg3\r\n");
	Parser::arguments_array	expect2;
	expect2.push_back(":prefix!pr@pr2");
	expect2.push_back("Command");
	expect2.push_back("Arg1");
	expect2.push_back("Arg2");
	expect2.push_back("Arg3");
	ASSERT_EQ(expect2, Parser::splitArgs(input2));

	std::string				input3("Command Arg1 Arg2 Arg3\r\n");
	Parser::arguments_array	expect3;
	expect3.push_back("Command");
	expect3.push_back("Arg1");
	expect3.push_back("Arg2");
	expect3.push_back("Arg3");
	ASSERT_EQ(expect3, Parser::splitArgs(input3));

	std::string				input4("\r\n");
	Parser::arguments_array	expect4;
	ASSERT_EQ(expect4, Parser::splitArgs(input4));


	std::string				input5(":prefix!pr@pr2 Command Arg1 Arg2 Arg3:Arg4 Arg5 :Arg6 :Arg7\r\n");
	Parser::arguments_array	expect5;
	expect5.push_back(":prefix!pr@pr2");
	expect5.push_back("Command");
	expect5.push_back("Arg1");
	expect5.push_back("Arg2");
	expect5.push_back("Arg3:Arg4");
	expect5.push_back("Arg5");
	expect5.push_back(":Arg6 :Arg7");
	ASSERT_EQ(expect5, Parser::splitArgs(input5));

	std::string				input6(":irc.example.net PASS  0210-IRC+ ngIRCd|26.1:CHLMSXZ PZ\r\n");
	Parser::arguments_array	expect6;
	expect6.push_back(":irc.example.net");
	expect6.push_back("PASS");
	expect6.push_back("0210-IRC+");
	expect6.push_back("ngIRCd|26.1:CHLMSXZ");
	expect6.push_back("PZ");
	ASSERT_EQ(expect6, Parser::splitArgs(input6));

}
#include "../src/command/Pass.cpp"
TEST(pass, inThisVersion) {
	std::string		input("12123213");
	ASSERT_EQ(true,isThisVersion(input));
	std::string		input1("121");
	ASSERT_EQ(false,isThisVersion(input1));
	std::string		input2("12154635634535");
	ASSERT_EQ(true,isThisVersion(input2));
	std::string		input3("1215463563453575465");
	ASSERT_EQ(false,isThisVersion(input3));
	std::string		input4("12a1653");
	ASSERT_EQ(false,isThisVersion(input4));
	std::string		input5("1216a53");
	ASSERT_EQ(true,isThisVersion(input5));
	std::string		input6("1216a53|");
	ASSERT_EQ(true,isThisVersion(input6));
}

TEST(pass, inThisFlag) {
	std::string		input("12123|213");
	ASSERT_EQ(true,isThisFlag(input));
	std::string		input1("121hdgs3566");
	ASSERT_EQ(false,isThisFlag(input1));
	std::string		input2("1");
	ASSERT_EQ(false,isThisFlag(input2));
	std::string		input3("|");
	ASSERT_EQ(true,isThisFlag(input3));
	std::string		input4("|asada");
	ASSERT_EQ(true,isThisFlag(input4));
	std::string		input5("1216a53|");
	ASSERT_EQ(true,isThisFlag(input5));
	std::string		input6("1216a53|dghdgh|hdh");
	ASSERT_EQ(false,isThisFlag(input6));
}

TEST(stringToSizeT, test) {
	size_t		dest = 0;

	EXPECT_TRUE(Parser::safetyStringToUl(dest, "123"));
	EXPECT_EQ(dest, 123);
	EXPECT_TRUE(Parser::safetyStringToUl(dest, "1231233142"));
	EXPECT_EQ(dest, 1231233142);
	EXPECT_TRUE(Parser::safetyStringToUl(dest, "0"));
	EXPECT_EQ(dest, 0);
	EXPECT_FALSE(Parser::safetyStringToUl(dest, "-123"));
	EXPECT_FALSE(Parser::safetyStringToUl(dest, "asdasdew"));
}

TEST(parser, copyStrFromChartoChar) {
	const std::string	str(":abc!def%gh");
	ASSERT_EQ(Parser::copyStrFromCharToChar(str, ':', 'a'), "");
	ASSERT_EQ(Parser::copyStrFromCharToChar(str, ':', '!'), "abc");
	ASSERT_EQ(Parser::copyStrFromCharToChar(str, ':', '%'), "abc!def");
	ASSERT_EQ(Parser::copyStrFromCharToChar(str, '%', ' '), "gh");
}

TEST(parser, isNumericString) {
	ASSERT_TRUE(Parser::isNumericString("1234567890"));
	ASSERT_TRUE(Parser::isNumericString("0"));
	ASSERT_TRUE(Parser::isNumericString("99999"));
	ASSERT_TRUE(Parser::isNumericString("00000"));
	ASSERT_FALSE(Parser::isNumericString(""));
	ASSERT_FALSE(Parser::isNumericString(" "));
	ASSERT_FALSE(Parser::isNumericString("123 "));
	ASSERT_FALSE(Parser::isNumericString("abcjasd"));
	ASSERT_FALSE(Parser::isNumericString("9999999!"));
}

TEST(parser, split) {
	std::string		src1("a,b,c,d");
	std::vector<std::string> res1({"a", "b", "c", "d"});
	ASSERT_EQ(res1, Parser::split(src1, ','));

	std::string		src2("a,b,c,d");
	std::vector<std::string> res2({"a,b,c,d"});
	ASSERT_EQ(res2, Parser::split(src2, ';'));

	std::string		src3("ab,cd");
	std::vector<std::string> res3({"ab", "cd"});
	ASSERT_EQ(res3, Parser::split(src3, ','));

	std::string		src4;
	std::vector<std::string> res4;
	ASSERT_EQ(res4, Parser::split(src4, ','));
}

TEST(mods, userChannelPriv) {
	Modes *	modes = UserChannelPrivileges::create();

	ASSERT_TRUE(modes->set(UserChannelPrivileges::mOperator));
	ASSERT_FALSE(modes->set('_'));

	ASSERT_TRUE(modes->check(UserChannelPrivileges::mOperator));
	ASSERT_FALSE(modes->check(UserChannelPrivileges::mCreator));
	ASSERT_FALSE(modes->check('_'));

	modes->unset(UserChannelPrivileges::mCreator);
	modes->unset(UserChannelPrivileges::mOperator);
	ASSERT_FALSE(modes->check(UserChannelPrivileges::mOperator));
	ASSERT_FALSE(modes->check(UserChannelPrivileges::mCreator));
	ASSERT_FALSE(modes->check('_'));

	ASSERT_TRUE(modes->parse("+oO-O"));
	ASSERT_TRUE(modes->checkAll("o"));
	ASSERT_FALSE(modes->checkAll("oO"));
	ASSERT_FALSE(modes->checkAll("O"));
	ASSERT_TRUE(modes->parse("-o"));
	ASSERT_FALSE(modes->checkAll("o"));
	ASSERT_TRUE(modes->parse("+O+o"));
	ASSERT_FALSE(modes->parse("+p"));
	ASSERT_TRUE(modes->checkAll("Oo"));
	delete modes;
}

#include "tools.hpp"
TEST(tools, time) {
	std::cout << tools::timeToString(time(nullptr)) << std::endl;
	std::cout << tools::timeToString(100) << std::endl;

	std::cout << tools::timeToString(tools::getModifyTime("ircserv")) << std::endl;
}
