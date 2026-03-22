/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _notice.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aehrl <aehrl@student.42malaga.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 21:22:49 by aehrl             #+#    #+#             */
/*   Updated: 2026/03/16 21:26:27 by aehrl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::parseNotice(Client &client)
{
    try{
        if(client.getFullmsg().size() < 2)
           return ; // Silently ignore
        if(client.getFullmsg().size() == 2)
        {
            if(client.getFullmsg()[1][0] == ':')
                throw ERR_NORECIPIENT;
            else
                throw ERR_NOTEXTTOSEND;
        }
        if(client.getFullmsg()[2][0] != ':')
            throw ERR_NOTEXTTOSEND;
        std::vector<std::string> name_vec = ft_split(client.getFullmsg()[1], ',', ':');
        for(size_t i = 0; i < name_vec.size(); i++)
        {
            std::string response;
            if(name_vec[i][0] != '#')
            {
				if(this->clients.end() != this->clients.find(getClientSocket(name_vec[i])))
                {
                    response += ":" + client.getHostname() + " NOTICE " + name_vec[i];
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
                        response += ":" + client.getHostname() + " NOTICE " + name_vec[i] + " ";
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