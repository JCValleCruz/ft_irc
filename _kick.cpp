/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _kick.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jormoral <jormoral@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/01 17:33:50 by jormoral          #+#    #+#             */
/*   Updated: 2025/09/18 14:24:26 by jormoral         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

///// kick #canal target  :reason            el target tiene Privilegios?
//     [0]   [1]    [2]     [3] 
// getKicked("reason", client);
// 		-> grupo 
// 		-> target
// 0(no esta) 1(esta) 2(esta y es mod)

void Server::parseKick(Client &client)
{
	try
	{
		std::vector<std::string> fullmsg = client.getFullmsg();
		std::string reason = "";
		if(fullmsg.size() < 3)
			throw(ERR_NEEDMOREPARAMS);
		int n = this->findChannelNumber(fullmsg[1]);
		if(n == -1)
			throw (ERR_NOSUCHCHANNEL);
		if(client.getNick() == fullmsg[2])
			throw (ERR_NOSELFKICK);
		if(this->channels[n].clientLevelInChannel(client.getNick()) == 0)//estamos in?
			throw (ERR_NOTONCHANNEL);
		if(this->channels[n].clientLevelInChannel(client.getNick()) == 1) //somos mods?
			throw (ERR_CHANOPRIVSNEEDED);
		if(fullmsg.size() == 4)
			reason = " " + fullmsg[3];
		if(this->channels[n].kickClient(fullmsg[2], client, reason) != 1)
			throw (ERR_USERNOTINCHANNEL);

	}
	catch(ERR num)
	{
		err(num, this->hostname, client);
	}
}