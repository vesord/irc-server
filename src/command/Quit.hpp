/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quit.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgarth <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/20 12:07:17 by cgarth            #+#    #+#             */
/*   Updated: 2021/01/20 12:07:18 by cgarth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ACommand.hpp"
#include "ServerInfo.hpp"
#include "Parser.hpp"
#include "ReplyList.hpp"

class Quit : public ACommand {
public:
	static const char * const		commandName;

	Quit(const std::string & commandLine, socket_type senderSocket, IServerForCmd & server);
	virtual ~Quit();

	static std::string			createReply(const std::string & reason);
	virtual replies_container	execute(IServerForCmd & server);
	static ACommand *			create(const std::string & commandLine,
									   socket_type senderSocket, IServerForCmd & server);

private:
    Quit();
    Quit(const Quit & other);
    Quit & operator= (const Quit & other);

    const static Parser::parsing_unit_type<Quit> _parsers[];

    Parser::parsing_result_type _commandNameParser(const IServerForCmd & server, const std::string & commandNameArgument);
    Parser::parsing_result_type _commentParser(const IServerForCmd & server, const std::string & commandNameArgument);

    bool		_isParamsValid(const IServerForCmd & server);
    void		_execute(IServerForCmd & server);

	std::string 	_cmd;
    std::string		_comment;
};

