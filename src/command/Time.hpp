/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Time.hpp                                           :+:      :+:    :+:   */
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

class Time : public ACommand {
public:
    static const char * const		commandName;

    Time(const std::string & commandLine, socket_type senderFd);

    static
    ACommand *	create(const std::string & commandLine, socket_type senderFd);
	static
	std::string	createTimeReply(const std::string & name);

    virtual replies_container	execute(IServerForCmd & server);

    ~Time();

private:
    Time();
    Time(const Time & other);
    Time & operator= (const Time & other);

    bool		_isPrefixValid(const IServerForCmd & server);
    bool		_isParamsValid(const IServerForCmd & server);
    void		_execute(IServerForCmd & server);

    std::string		_server;
};

