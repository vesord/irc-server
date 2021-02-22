/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Oper.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zkerriga <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/20 12:07:17 by zkerriga          #+#    #+#             */
/*   Updated: 2021/01/20 12:07:24 by zkerriga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Oper.hpp"
#include "Mode.hpp"
#include "BigLogger.hpp"
#include "ReplyList.hpp"
#include "Configuration.hpp"
#include "IClient.hpp"

#include <vector>

Oper::Oper() : ACommand("", "", 0, nullptr) {}
Oper::Oper(const Oper & other) : ACommand("", "", 0, nullptr) {
	*this = other;
}
Oper & Oper::operator=(const Oper & other) {
	if (this != &other) {}
	return *this;
}

Oper::~Oper() {}

Oper::Oper(const std::string & commandLine,
			 const socket_type senderSocket, IServerForCmd & server)
	: ACommand(commandName, commandLine, senderSocket, &server) {}

ACommand *Oper::create(const std::string & commandLine,
						socket_type senderFd, IServerForCmd & server) {
	return new Oper(commandLine, senderFd, server);
}

const char * const	Oper::commandName = "OPER";

/**
 * @author matrus
 * @brief checks if name and password are registered on server
 * and if yes, registers Client as Operator.
 *
 * @warning this OPER does not support ERR_NOOPERHOST reply.
 * */

ACommand::replies_container Oper::execute(IServerForCmd & server) {
	BigLogger::cout(std::string(commandName) + ": execute");
	if (_isParamsValid(server)) {
		_execute(server);
	}
	return _commandsToSend;
}

const Parser::parsing_unit_type<Oper>	Oper::_parsers[] = {
	{.parser=&Oper::_prefixParser, .required=false},
	{.parser=&Oper::_commandNameParser, .required=true},
	{.parser=&Oper::_nameParser, .required=true},
	{.parser=&Oper::_passwordParser, .required=true},
	{.parser=nullptr, .required=false}
};

void Oper::_execute(IServerForCmd & server) {

	IClient * clientOnFd = server.findNearestClientBySocket(_senderSocket);
	if (clientOnFd) {
		_executeForClient(server, clientOnFd);
		return;
	}

	if (server.findRequestBySocket(_senderSocket)) {
		BigLogger::cout(std::string(commandName) + ": discard: got from request", BigLogger::YELLOW);
		return;
	}

	if (server.findNearestServerBySocket(_senderSocket)) {
		BigLogger::cout(std::string(commandName) + ": discard: got from server", BigLogger::YELLOW);
		return;
	}

	BigLogger::cout(std::string(commandName) + ": UNRECOGNIZED CONNECTION DETECTED! CONSIDER TO CLOSE IT.", BigLogger::RED);
	server.forceCloseConnection_dangerous(_senderSocket, "");
}

void Oper::_executeForClient(IServerForCmd & server, IClient * client) {
	if (!server.getConfiguration().isOperator(_name, _password)) {
		_addReplyToSender(server.getPrefix() + " " + errPasswdMismatch("*"));
		return;
	}
	client->setPrivilege(UserMods::mOperator);
	_addReplyToSender(server.getPrefix() + " " + rplYouReOper("*"));
	_createAllReply(server, server.getPrefix() + " " + Mode::createReply(client));
}

bool Oper::_isParamsValid(IServerForCmd & server) {
	return Parser::argumentsParser(server,
							Parser::splitArgs(_rawCmd),
							Oper::_parsers,
							this,
							_commandsToSend[_senderSocket]);
}

void Oper::_createAllReply(const IServerForCmd & server, const std::string & reply) {
	typedef IServerForCmd::sockets_set				sockets_container;
	typedef sockets_container::const_iterator		iterator;

	const sockets_container		sockets = server.getAllServerConnectionSockets();
	iterator					ite = sockets.end();

	for (iterator it = sockets.begin(); it != ite; ++it) {
		if (*it != _senderSocket) {
			_addReplyTo(*it, reply);
		}
	}
}

Parser::parsing_result_type Oper::_prefixParser(const IServerForCmd & server,
												const std::string & prefixArgument) {
	_fillPrefix(_rawCmd);
	if (!_prefix.name.empty()) {
		if (!(
			server.findClientByNickname(_prefix.name)
			|| server.findServerByName(_prefix.name))) {
			return Parser::CRITICAL_ERROR;
		}
	}
	return Parser::SKIP_ARGUMENT;
}

Parser::parsing_result_type Oper::_commandNameParser(const IServerForCmd & server,
													 const std::string & commandNameArgument) {
	if (Parser::toUpperCase(commandNameArgument) != commandName) {
		return Parser::CRITICAL_ERROR;
	}
	return Parser::SUCCESS;
}

Parser::parsing_result_type Oper::_nameParser(const IServerForCmd & server,
											  const std::string & nameArgument) {
	_name = nameArgument;
	return Parser::SUCCESS;
}

Parser::parsing_result_type Oper::_passwordParser(const IServerForCmd & server,
												  const std::string & passwordArgument) {
	_password = passwordArgument;
	return Parser::SUCCESS;
}
