/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zkerriga <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/25 22:59:46 by zkerriga          #+#    #+#             */
/*   Updated: 2021/01/25 22:59:47 by zkerriga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tools.hpp"

static void		prepareSocketToListen(const socket_type listener) {
	static const int	maxPossibleConnections = 10;
	if (listen(listener, maxPossibleConnections) < 0) {
		throw std::runtime_error(std::string("listen: ") + strerror(errno));
	}
}

socket_type		tools::configureListenerSocket(const int port) {
	socket_type			listener = 0;
	struct addrinfo		hints;
	struct addrinfo *	ai;
	int					ret = 0;

	std::memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((ret = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &ai)) != 0) {
		throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(ret));
	}
	addrinfo *	i;
	for (i = ai; i != nullptr; i = i->ai_next) {
		listener = socket(i->ai_family, SOCK_STREAM, getprotobyname("tcp")->p_proto);
		if (listener < 0) {
			continue;
		}
		int		yes = 1;
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
			continue;
		}
		if (bind(listener, i->ai_addr, i->ai_addrlen) < 0) {
			close(listener);
			continue;
		}
		/* todo: log set configure ip4/ip6 */
		break;
	}
	if (i == nullptr) {
		throw std::runtime_error("select server: failed to bind");
	}
	freeaddrinfo(ai);
	prepareSocketToListen(listener);
	return listener;
}
