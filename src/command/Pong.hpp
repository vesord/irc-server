/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pong.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zkerriga <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/20 12:07:17 by zkerriga          #+#    #+#             */
/*   Updated: 2021/01/20 12:07:18 by zkerriga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

#include "ACommand.hpp"
#include "Parser.hpp"
#include "ReplyList.hpp"
#include "ServerInfo.hpp"

class Pong : public ACommand {
public:

	static const char *		commandName;

	virtual ~Pong();

	Pong(const std::string & commandLine, socket_type senderFd);

	static
	ACommand *	create(const std::string & commandLine, socket_type senderFd);

	virtual replies_container	execute(IServerForCmd & server);

private:

	bool	_isPrefixValid(const IServerForCmd & server);
	bool	_isParamsValid(IServerForCmd & server);
	void	_execute(IServerForCmd & server);

	std::string 	_target;
	std::string 	_token;

	Pong();
	Pong(const Pong & other);
	Pong & operator= (const Pong & other);


};
