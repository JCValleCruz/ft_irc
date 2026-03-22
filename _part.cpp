/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _part.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aehrl <aehrl@student.42malaga.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:50:41 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/22 19:33:04 by aehrl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::parsePart(Client &client)
{
    try
	{
        std::string response = ":" + client.getHostname() + " PART ";
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
		
			this->channels[n].removeClient(client);
			checkBotShouldLeave(this->channels[n]);
			nextModerator(client.getNick(), this->channels[n]);
            if (this->channels[n].getClients().size() == 1 &&  this->channels[n].getClients()[0].getNick() == "GabiBot")
				deleteChannel(this->channels[n].getName());
			else{
			
				std::vector<Client> aux= this->channels[n].getClients();
				std::string info_msg = ":" + gethostName() + " 353 " + client.getNick() + " = " + channels[n].getName() + " :";
				std::string client_names = "";
				for(size_t i = 0; i < aux.size(); i++)
				{
					if(this->channels[n].isAMod(aux[i].getNick()))
						client_names += "@";

					client_names += aux[i].getNick();

					if(i + 1 != aux.size())
						client_names += " ";
				}
				info_msg += client_names;
				client.setResponse(client.getResponse() + info_msg + "\r\n"); 
				std::string endofnames = ":" + gethostName() + " 366 " + client.getNick() + " " + channels[n].getName() + " :End of /NAMES list.";
				client.setResponse(client.getResponse() + endofnames); client.sendResponse();

				this->channels[n].sendResponseChannel(response + this->channels[n].getName() + reason, client, 0);
			}
        }
	}
	catch(ERR num)
	{
		err(num, this->hostname, client);
	}
}