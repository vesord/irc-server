/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Version.hpp                                        :+:      :+:    :+:   */
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

class Version : public ACommand {
public:
	Version();
	Version(const Version & other);
	~Version();
	Version & operator= (const Version & other);

	static
	ACommand *	create() {
		return new Version();
	}
private:

};

