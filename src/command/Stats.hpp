/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Stats.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zkerriga <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/20 12:07:17 by zkerriga          #+#    #+#             */
/*   Updated: 2021/01/20 12:07:18 by zkerriga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ACommand.hpp"
#include "ServerInfo.hpp"
#include "Parser.hpp"
#include "ReplyList.hpp"

class Stats : public ACommand {
public:
	static const char * const		commandName;

	Stats(const std::string & commandLine, socket_type senderFd);

	static
	ACommand *	create(const std::string & commandLine, socket_type senderFd);

	virtual replies_container	execute(IServerForCmd & server);

	~Stats();

private:
	Stats();
	Stats(const Stats & other);
	Stats & operator=(const Stats & other);

	void		_execute(IServerForCmd & server);
	bool		_isParamsValid(IServerForCmd & server);
	void		_sendStats(IServerForCmd & server);
	std::string _createRawReply();

	/// QUERIES

	struct query_processor_t {
		const char	query;
		std::string (Stats::*reply)(const IServerForCmd & server);
	};

	static const query_processor_t _queries[];

	std::string	_generateLinksInfoRpl(const IServerForCmd & server);
	std::string	_generateCommandsRpl(const IServerForCmd & server);
	std::string	_generateUptimeRpl(const IServerForCmd & server);
	std::string	_generateOpersRpl(const IServerForCmd & server);
	std::string	_generateEasterEggRpl(const IServerForCmd & server);

	/// PARSING

	static const Parser::parsing_unit_type<Stats> _parsers[];

	Parser::parsing_result_type _commandNameParser(const IServerForCmd & server, const std::string & commandNameArg);
	Parser::parsing_result_type _queryParser(const IServerForCmd & server, const std::string & maskArg);
	Parser::parsing_result_type _targetParser(const IServerForCmd & server, const std::string & targetArg);

	std::string     _query;
	std::string		_target;
};


