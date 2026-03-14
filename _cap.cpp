/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _cap.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:46:56 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/14 12:56:16 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::parseCap(Client &client)
{
	try
	{
		if(client.getFullmsg().size() == 1)
			throw(ERR_NEEDMOREPARAMS);
		if(client.getMessage() != "CAP LS" && client.getMessage() != "CAP LS 302")
			throw(ERR_UNKNOWNERROR);
		client.setResponse("CAP * LS");
	}
	catch(ERR num)
	{
		err(num, this->hostname, client,"Command CAP must be followed by LS or LS 302");
	}
}