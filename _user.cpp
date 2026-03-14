/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _user.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 13:32:27 by jose-rig          #+#    #+#             */
/*   Updated: 2025/09/29 19:47:42 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::parseUser(Client &client)
{
	try
	{
		if(client.getNick() == "")
			throw ERR_NOTREGISTERED;
		std::vector<std::string> fullmsg = client.getFullmsg();
		if(fullmsg.size() != 5)
		{
			throw(ERR_NEEDMOREPARAMS);
					
		}
		
		else if(client.getUserVerified() == true || userInUse(fullmsg[1]))
			throw(ERR_ALREADYREGISTERED);
		else if(client.getVerify() == false)
			throw(ERR_NOTREGISTERED);
		else if(fullmsg[3] != "*" || fullmsg[2] != "0" || fullmsg[4][0] == '\0')
			throw ERR_NEEDMOREPARAMS;
		else
			client.setUsername(fullmsg[1]);
	}
	catch(ERR num)
	{
		err(num, this->hostname, client);
	}
}

int Server::userInUse(std::string user)
{
	std::map<int, Client >::iterator it = this->clients.begin();
	std::map<int, Client >::iterator ite = this->clients.end();
	while(it != ite)
	{
		if((*it).second.getUsername() == user)
			return 1;
		it++;
	}
	return(0);
}