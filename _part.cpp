/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _part.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sbenitez <sbenitez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:50:41 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/22 16:15:46 by sbenitez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
            if(this->channels[n].clientLevelInChannel(client.getNick()) == 0)
                throw ERR_NOTONCHANNEL;
			nextModerator(client.getNick(), this->channels[n]);
			this->channels[n].removeClient(client);

			checkBotShouldLeave(this->channels[n]);

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