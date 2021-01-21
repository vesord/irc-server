/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ACommand.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zkerriga <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/20 11:59:31 by zkerriga          #+#    #+#             */
/*   Updated: 2021/01/20 11:59:32 by zkerriga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <list>
#include <map>
#include <string>
#include "ServerInfo.hpp"

class Server;

class ACommand {
	typedef std::list<std::string> reply_args_type;

public:

	struct pair_code_fuction {
		int	code;
		std::string (*function)(std::list<std::string>);
	};
	static const pair_code_fuction _replyList[];

	typedef struct	command_prefix_s {
		std::string name;
		std::string user;
		std::string host;
		std::string toString() const {
			std::string ret = ":" + name;
			ret += user.empty() ? "" : "!" + user;
			ret += host.empty() ? "" : "@" + host;
			return ret;
		};
	}				command_prefix_t;
	typedef int socket_type;
	typedef std::map<socket_type, std::string>	replies_container;

	ACommand(const std::string & rawCmd, int senderFd);

	~ACommand();

	virtual replies_container 	execute(Server & server) = 0;

protected:

//	virtual bool		_isSyntaxCorrect() = 0;
//	virtual bool		_isAllParamsCorrect(Server & server) = 0;
//	virtual void		_execute(Server & server) = 0;

	void				_reply(int code, reply_args_type args);

	const std::string	_rawCmd;
	const int			_senderFd;
	bool				_needDiscard;
	replies_container 	_commandsToSend;

	struct _prefix {

	};
	std::string 		_commandName;
	std::string 		_params;
	command_prefix_t	_prefix;


private:
	ACommand();
	ACommand( const ACommand & aCommand);
	ACommand & operator=(const ACommand & aCommand);

};
