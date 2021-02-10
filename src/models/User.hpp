/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zkerriga <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/19 09:56:07 by zkerriga          #+#    #+#             */
/*   Updated: 2021/01/19 09:56:14 by zkerriga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

#include "IClient.hpp"
#include "ServerInfo.hpp"
#include "IMods.hpp"

class User : public IClient {
public:
	User(socket_type sokcet, const std::string & nick, const Configuration & conf);
	User(socket_type socket,
		 const std::string & nick,
		 size_t	hopcount,
		 const std::string & username,
		 const std::string & host,
		 size_t serverToken,
		 const std::string & uMode,
		 const std::string & realName,
		 const ServerInfo * serverInfo,
		 const Configuration & conf);

		virtual ~User();

	virtual time_t				getLastReceivedMsgTime() const;
	virtual size_t				getHopCount() const;
	virtual	time_t				getTimeout() const;
	virtual	const std::string &	getName() const;
	virtual socket_type			getSocket() const;

	virtual void		setReceivedMsgTime();

	virtual bool		changeName(const std::string & name);


private:
	User();
	User(const User & other);
	User & operator= (const User & other);

	static const time_t	c_defaultTimeoutForRequestSec = 3;

	socket_type			_socket;
	std::string			_nick;
	size_t				_hopCount;
	std::string			_username;
	std::string			_host;
	size_t				_serverToken;
	std::string 		_rawModes;
	std::string			_realName;
	const ServerInfo *	_server;

	std::string			_id;
	IMods *				_modes;

	time_t				_lastReceivedMsgTime;
	time_t				_timeout;
};
