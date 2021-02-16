/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zkerriga <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/20 12:07:17 by zkerriga          #+#    #+#             */
/*   Updated: 2021/01/20 12:07:24 by zkerriga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Mode.hpp"
#include "IClient.hpp"
#include "Join.hpp"
#include "tools.hpp"
#include "BigLogger.hpp"
#include "ReplyList.hpp"

Mode::Mode() : ACommand("", 0), _modes(Modes(""))  {}
Mode::Mode(const Mode & other) : ACommand("", 0), _modes(Modes("")) {
	*this = other;
}
Mode & Mode::operator=(const Mode & other) {
	if (this != &other) {}
	return *this;
}


Mode::~Mode() {
	/* todo: destructor */
}

Mode::Mode(const std::string & rawCmd, socket_type senderFd)
	: ACommand(rawCmd, senderFd), _modes(Modes(""))
{}

ACommand * Mode::create(const std::string & commandLine, const socket_type senderFd) {
	return new Mode(commandLine, senderFd);
}

const char * const		Mode::commandName = "MODE";
const char				set = '+';
const char				del = '-';

/**
 * \author matrus
 * \related RFC 2812: main reference
 */

ACommand::replies_container Mode::execute(IServerForCmd & server) {
	BigLogger::cout(std::string(commandName) + ": execute.");

	if (server.findRequestBySocket(_senderFd)) {
		BigLogger::cout(std::string(commandName) + ": discard: got from request");
		return _commandsToSend;
	}

	if (_isParamsValid(server)) {
		_execute(server);
	}
	return _commandsToSend;
}

const Parser::parsing_unit_type<Mode> Mode::_parsers[] = {
	{.parser = &Mode::_prefixParser, .required = false},
	{.parser = &Mode::_commandNameParser, .required = true},
	{.parser = &Mode::_targetParser, .required = true},
	{.parser = &Mode::_modesParser, .required = true},
	{.parser = &Mode::_paramParser, .required = false},
	{.parser = &Mode::_paramParser, .required = false},
	{.parser = &Mode::_paramParser, .required = false},
	{.parser = nullptr, .required = false}
};

Parser::parsing_result_type Mode::_prefixParser(const IServerForCmd & server, const std::string & prefixArg) {
	Parser::fillPrefix(_prefix, prefixArg);
	if (!_prefix.name.empty()) {
		if (!(
			server.findClientByNickname(_prefix.name)
			|| server.findServerByServerName(_prefix.name))) {
			return Parser::SUCCESS;
		}
		BigLogger::cout(std::string(commandName) + ": discard: prefix unknown", BigLogger::YELLOW);
		return Parser::CRITICAL_ERROR;
	}
	const IClient * client = server.findNearestClientBySocket(_senderFd);
	if (client) {
		_prefix.name = client->getName();
		_prefix.host = client->getHost();
		_prefix.user = client->getUsername();
		return Parser::SKIP_ARGUMENT;
	}
	BigLogger::cout(std::string(commandName) + ": discard: no prefix form connection", BigLogger::YELLOW);
	return Parser::CRITICAL_ERROR;
}

Parser::parsing_result_type
Mode::_commandNameParser(const IServerForCmd & server,
						 const std::string & prefixArg) {
	if (Parser::toUpperCase(prefixArg) == commandName) {
		return Parser::SUCCESS;
	}
	return Parser::CRITICAL_ERROR;
}

Parser::parsing_result_type Mode::_targetParser(const IServerForCmd & server,
												const std::string & targetArg) {
	_targetChannelOrNickname = targetArg;
	return Parser::SUCCESS;
}

Parser::parsing_result_type
Mode::_modesParser(const IServerForCmd & server, const std::string & modesArg) {
	_rawModes = modesArg;
	return Parser::SUCCESS;
}

Parser::parsing_result_type
Mode::_paramParser(const IServerForCmd & server, const std::string & param1Arg) {
	for (int i = 0; i < c_modeMaxParams; ++i) {
		if (_params[i].empty()) {
			_params[i] = param1Arg;
			return Parser::SUCCESS;
		}
	}
	BigLogger::cout(std::string(commandName) + ": discard: too much args for parsing");
	return Parser::CRITICAL_ERROR;
}

bool Mode::_isParamsValid(IServerForCmd & server) {
	return Parser::argumentsParser(server,
								Parser::splitArgs(_rawCmd),
								_parsers,
								   this,
								   _commandsToSend[_senderFd]);
}

void Mode::_execute(IServerForCmd & server) {
	/* Client name will never match channel name (cos of #) */

	IChannel * channel = server.findChannelByName(_targetChannelOrNickname);
	if (channel) {
		_executeForChannel(server, channel);
		return;
	}

	IClient * client = server.findClientByNickname(_targetChannelOrNickname);
	if (client) {
		if (Parser::toUpperCase(client->getName()) == Parser::toUpperCase(_targetChannelOrNickname)) {
			_executeForClient(server, client);
		}
		else {
			BigLogger::cout(std::string(commandName) + ": error: nick mismatch");
			_addReplyToSender(server.getServerPrefix() + " " + errUsersDontMatch());
		}
		return;
	}

	BigLogger::cout(std::string(commandName) + ": error: target not found");
	const std::string errRpl = Join::isValidChannel(_targetChannelOrNickname)
							   ? errNoSuchChannel(_targetChannelOrNickname)
							   : errNoSuchNick(_targetChannelOrNickname);
	_addReplyToSender(server.getServerPrefix() + " " + errRpl);
}

void Mode::_executeForClient(IServerForCmd & server, IClient * client) {
	std::string::size_type pos;
	setModesErrors ret = _trySetModesToClient(server, client, pos);

	if (ret == Mode::UNKNOWNMODE) {
		_addReplyToSender(server.getServerPrefix() + " " + errUModeUnknownFlag());
	}
	_addReplyToSender(server.getServerPrefix() + " " + rplUModeIs(client->getUMode()));
}

void Mode::_executeForChannel(IServerForCmd & server,
							  const IChannel * channel) {

}

void Mode::_createAllReply(const IServerForCmd & server, const std::string & reply) {
	typedef IServerForCmd::sockets_set				sockets_container;
	typedef sockets_container::const_iterator		iterator;

	const sockets_container		sockets = server.getAllServerConnectionSockets();
	iterator					ite = sockets.end();

	for (iterator it = sockets.begin(); it != ite; ++it) {
		if (*it != _senderFd) {
			_commandsToSend[*it].append(reply);
		}
	}
}

// aiwroOs
const Mode::map_mode_fuction<IClient *> Mode::_mapModeSetClient[] = {
	{.mode = 'a', .modeSetter =&Mode::_trySetClient_a},
	{.mode = 'i', .modeSetter = &Mode::_trySetClient_i},
	{.mode = 'w', .modeSetter = &Mode::_trySetClient_w},
	{.mode = 'r', .modeSetter = &Mode::_trySetClient_r},
	{.mode = 'o', .modeSetter = &Mode::_trySetClient_o},
//	{.mode = 'O', .modeSetter = &Mode::_trySetClient_O},
//	{.mode = 's', .modeSetter = &Mode::_trySetClient_s},
	{.mode = '\0', .modeSetter = nullptr}
};

// aimnqpsrtklbeI Oov
const Mode::map_mode_fuction<IChannel *> Mode::_mapModeSetChannel[] = {
//	{.mode = 'a', .modeSetter = &Mode::_trySetChannel_a},
//	{.mode = 'i', .modeSetter = &Mode::_trySetChannel_i},
//	{.mode = 'm', .modeSetter = &Mode::_trySetChannel_m},
//	{.mode = 'n', .modeSetter = &Mode::_trySetChannel_n},
//	{.mode = 'q', .modeSetter = &Mode::_trySetChannel_q},
//	{.mode = 'p', .modeSetter = &Mode::_trySetChannel_p},
//	{.mode = 's', .modeSetter = &Mode::_trySetChannel_s},
//	{.mode = 'r', .modeSetter = &Mode::_trySetChannel_r},
//	{.mode = 't', .modeSetter = &Mode::_trySetChannel_t},
//	{.mode = 'k', .modeSetter = &Mode::_trySetChannel_k},
//	{.mode = 'l', .modeSetter = &Mode::_trySetChannel_l},
//	{.mode = 'b', .modeSetter = &Mode::_trySetChannel_b},
//	{.mode = 'e', .modeSetter = &Mode::_trySetChannel_e},
//	{.mode = 'I', .modeSetter = &Mode::_trySetChannel_I},
//	{.mode = 'O', .modeSetter = &Mode::_trySetChannel_O},
//	{.mode = 'o', .modeSetter = &Mode::_trySetChannel_o},
//	{.mode = 'v', .modeSetter = &Mode::_trySetChannel_v},
	{.mode = '\0', .modeSetter = nullptr},
};


Mode::setModesErrors
Mode::_trySetModesToClient(const IServerForCmd & server, IClient * client,
						   std::string::size_type & pos) {
	const std::string::size_type	size = _rawModes.size();
	char							action = '\0';
	int								j = 0;
	setModesErrors					ret;

	if (_rawModes[pos] != set && _rawModes[pos] != del) {
		return Mode::FAIL;
	}
	action = _rawModes[pos];

	for (; pos < size; ++pos) {
		if (_rawModes[pos] == set || _rawModes[pos] == del) {
			action = _rawModes[pos];
			continue;
		}
		for (j = 0; _mapModeSetClient[j].modeSetter != nullptr; ++j) {
			if (_mapModeSetClient[j].mode != _rawModes[pos]) {
				continue;
			}
			if ( (ret = (this->*(_mapModeSetClient[j].modeSetter))(server, client, action == set) ) != Mode::SUCCESS ) {
				if (ret == Mode::FAIL) {
					continue;
				}
				return ret;
			}
			break;
		}
		if (_mapModeSetClient[j].modeSetter == nullptr) {
			return Mode::UNKNOWNMODE;
		}
	}
	return Mode::SUCCESS;
}

Mode::setModesErrors
Mode::_trySetClient_a(const IServerForCmd & serer, IClient * client, bool isSet) {
	/* SHALL NOT be toggled by the user using the MODE command,
	 * instead use of the AWAY command is REQUIRED // RFC 2812 3.1.5 */
	return Mode::FAIL;
}

Mode::setModesErrors
Mode::_trySetClient_i(const IServerForCmd & serer, IClient * client,
					  bool isSet) {
	const char mode = 'i';
	if (isSet) {
		if (client->setPrivilege(mode)) {
			return Mode::SUCCESS;
		}
		return Mode::FAIL;
	}
	else {
		client->unsetPrivilege(mode);
	}
	return Mode::SUCCESS;
}

Mode::setModesErrors
Mode::_trySetClient_w(const IServerForCmd & serer, IClient * client,
					  bool isSet) {
	const char mode = 'w';
	/* User can not receive wallops */
	return Mode::FAIL;
}

Mode::setModesErrors
Mode::_trySetClient_r(const IServerForCmd & serer, IClient * client,
					  bool isSet) {
	const char mode = 'r';
	if (isSet) {
		if (client->setPrivilege(mode)) {
			return Mode::SUCCESS;
		}
		return Mode::FAIL;
	}
	return Mode::FAIL;
}

Mode::setModesErrors
Mode::_trySetClient_o(const IServerForCmd & serer, IClient * client,
					  bool isSet) {
	const char mode = 'o';
	if (isSet) {
		return Mode::FAIL;
	}
	else {
		client->unsetPrivilege(mode);
	}
	return Mode::SUCCESS;
}
