/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zkerriga <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/20 12:07:17 by zkerriga          #+#    #+#             */
/*   Updated: 2021/01/20 12:07:18 by zkerriga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>

#include "ACommand.hpp"
#include "Parser.hpp"
#include "ReplyList.hpp"

class Join : public ACommand {
public:
	static const char * const	commandName;

	Join(const std::string & commandLine, socket_type senderSocket, IServerForCmd & server);
	virtual ~Join();

	static ACommand *			create(const std::string & commandLine,
									   socket_type senderSocket, IServerForCmd & server);
	virtual replies_container	execute(IServerForCmd & server);

	static bool isValidChannel(const std::string & name);

private:
	Join();
	Join(const Join & other);
	Join & operator= (const Join & other);

	bool		_parsingIsPossible();
	void		_executeChannel(const std::string & channel, const std::string & key);
	std::string	_createMessageToServers(const std::string & channelWithModes,
										const std::string & key) const;
	std::string	_createNotifyForMembers(const std::string & channel) const;
	void		_sendTopicAfterJoin(IChannel * channel);

	static const Parser::parsing_unit_type<Join>	_parsers[];
	Parser::parsing_result_type	_prefixParser(const std::string & prefixArgument);
	Parser::parsing_result_type	_channelsParser(const std::string & channelsArgument);
	Parser::parsing_result_type	_passwordsParser(const std::string & passwordsArgument);

	typedef std::pair<std::string, std::string>	channel_pair;
	typedef std::vector<channel_pair>			container;
	container	_channels;
	IClient *	_client;
};
