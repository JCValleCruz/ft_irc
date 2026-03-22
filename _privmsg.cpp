/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _privmsg.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sbenitez <sbenitez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 16:50:22 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/22 16:16:39 by sbenitez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Bot.hpp"

void Server::parsePrivmsg(Client &client)
{
    try{
        if(client.getFullmsg().size() == 1)
            throw ERR_NEEDMOREPARAMS;
        if(client.getFullmsg().size() == 2)
        {
            if(client.getFullmsg()[1][0] == ':')
                throw ERR_NORECIPIENT;
            else
                throw ERR_NOTEXTTOSEND;
        }
        if(client.getFullmsg()[2][0] != ':')
            throw ERR_NOTEXTTOSEND;

		if (client.getFullmsg()[2].size() > 1 && client.getFullmsg()[2][1] == '!')		/**/
		{																				/**/
			Bot::handleCommand(client, *this);											/**/
			return;																		/**/
		}

        std::vector<std::string> name_vec = ft_split(client.getFullmsg()[1], ',', ':');
        for(size_t i = 0; i < name_vec.size(); i++)
        {
            std::string response;
            if(name_vec[i][0] != '#')
            {
				if(this->clients.end() != this->clients.find(getClientSocket(name_vec[i])))
                {
                    response += ":" + client.getHostname() + " PRIVMSG " + name_vec[i];
                    response += " " + client.getFullmsg()[2] + "\r\n";
                    send(getClientSocket(name_vec[i]), response.c_str(), response.size(), AF_INET);
                    send(getClientSocket(client.getNick()), response.c_str(), response.size(), AF_INET);
                }
                else
                    throw ERR_NOSUCHNICK;
            }  
            else
            {
                if(findChannelNumber(name_vec[i]) != -1)
                {
                    if(!this->channelExists(name_vec[i]))
                        throw ERR_NOSUCHCHANNEL;
                    int channel_index = findChannelNumber(name_vec[i]);
                    Channel &temp =this->channels[channel_index];
                    if(temp.isInChannel(client.getNick()) == 0)
                        throw ERR_CANNOTSENDTOCHAN;
					else
                    {
                        response += ":" + client.getHostname() + " PRIVMSG " + name_vec[i] + " ";
                        response += client.getFullmsg()[2];
                        temp.sendResponseChannel(response, client, 1);
                    }
                }
			}
        }
    }
    catch(ERR num)
	{
		err(num, this->hostname, client);
	}
	
}