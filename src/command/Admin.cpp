/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Admin.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgarth <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/20 12:07:17 by cgarth            #+#    #+#             */
/*   Updated: 2021/01/20 12:07:24 by cgarth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Admin.hpp"
#include "BigLogger.hpp"
#include "IClient.hpp"
#include "tools.hpp"
#include "debug.hpp"
#include "Server.hpp"

Admin::Admin() : ACommand("", "", 0, nullptr) {}
Admin::Admin(const Admin & other) : ACommand("", "", 0, nullptr) {
	*this = other;
}
Admin & Admin::operator=(const Admin & other) {
	if (this != &other) {}
	return *this;
}

Admin::~Admin() {}

Admin::Admin(const std::string & commandLine,
			 const socket_type senderSocket, IServerForCmd & server)
	: ACommand(commandName, commandLine, senderSocket, &server) {}

ACommand *Admin::create(const std::string & commandLine,
						socket_type senderFd, IServerForCmd & server) {
	return new Admin(commandLine, senderFd, server);
}

const char * const	Admin::commandName = "ADMIN";
#define CMD std::string(commandName)

std::string Admin::createAdminReply(const std::string & name) {
	return CMD + " " + name + Parser::crlf;
}

const Parser::parsing_unit_type<Admin>	Admin::_parsers[] = {
		{.parser=&Admin::_defaultPrefixParser, .required=false},
		{.parser=&Admin::_defaultCommandNameParser, .required=true},
		{.parser=&Admin::_targetParser, .required=false},
		{.parser=nullptr, .required=false}
};

ACommand::replies_container Admin::execute(IServerForCmd & server) {
	if (_parsingIsPossible()) {
		if (_targets.empty()) {
			_targets.push_front(server.getSelfServerInfo());
		}
		_execute();
	}
	return _commandsToSend;
}

bool Admin::_parsingIsPossible() {
	return Parser::argumentsParser(
			*_server,
			Parser::splitArgs(_rawCmd),
			_parsers,
			this,
			_commandsToSend[_senderSocket]
	);
}

Parser::parsing_result_type
Admin::_targetParser(const std::string & targetArgument) {
	_targets = _server->getAllServerInfoForMask(targetArgument);
	if (_targets.empty()) {
		_addReplyToSender(_server->getPrefix() + " " + errNoSuchServer(_prefix.name, targetArgument));
		return Parser::CRITICAL_ERROR;
	}
	DEBUG3(BigLogger::cout(std::string(CMD + ": _targetParser: success -> size = ") + _targets.size(), BigLogger::YELLOW);)
	return Parser::SUCCESS;
}

void Admin::_execute() {
	DEBUG3(BigLogger::cout(CMD + ": valid -> _execute", BigLogger::YELLOW);)
	const std::string &		selfServerName = _server->getName();

	const ServerInfo *		target = _targets.front();
	if (selfServerName == target->getName()) {
		const std::string	prefix = _server->getPrefix() + " ";

		_addReplyToSender(prefix + rplAdminMe(_prefix.name, _server->getName()));
		_addReplyToSender(prefix + rplAdminLoc1(_prefix.name, "Description"));
		_addReplyToSender(prefix + rplAdminLoc2(_prefix.name, "Location"));
		_addReplyToSender(prefix + rplAdminEmail(_prefix.name, "admin@irc.server"));
	}
	else {
		_addReplyTo(
			target->getSocket(),
			_prefix.toString() + " " + createAdminReply(target->getName())
		);
	}
}

#undef CMD
