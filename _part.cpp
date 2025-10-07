/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _part.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jormoral <jormoral@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:50:41 by jormoral          #+#    #+#             */
/*   Updated: 2025/09/22 12:50:43 by jormoral         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

// PART #canal :reason
//    0   1       2
//siz 1   2       80
void Server::parsePart(Client &client)
{
    try
	{
        std::string response = ":" + client.getHostname() + " HAS LEFT ";
		std::vector<std::string> fullmsg = client.getFullmsg();
		std::string reason = "";
		if(fullmsg.size() < 2 || fullmsg.size() > 3)
			throw ERR_NEEDMOREPARAMS;
        if(fullmsg.size() == 3)
            reason = " :" + fullmsg[2];
        std::vector<std::string> vec_channel = ft_split(fullmsg[1], ',', '\0');
        for(size_t i = 0; i < vec_channel.size(); i++)
        {
            int n = this->findChannelNumber(vec_channel[i]);
            if(n == -1)
                throw ERR_NOSUCHCHANNEL;
            if(this->channels[n].clientLevelInChannel(client.getNick()) == 0)//esta en el canal?
                throw ERR_NOTONCHANNEL;
			nextModerator(client.getNick(), this->channels[n]);
			this->channels[n].removeClient(client);
            if(this->channels[n].getClients().size() == 0)
				deleteChannel(this->channels[n].getName());
			else
				this->channels[n].sendResponseChannel(response + this->channels[n].getName() + reason, client, 0);
        }
	}
	catch(ERR num)
	{
		err(num, this->hostname, client);
	}
}