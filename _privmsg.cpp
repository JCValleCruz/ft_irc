/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _privmsg.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jose-rig <jose-rig@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 16:50:22 by jormoral          #+#    #+#             */
/*   Updated: 2025/09/18 15:50:32 by jose-rig         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/// Privmsg target,#channel :texto
/* :Angel PRIVMSG Wiz :Hello are you receiving this message ?
    ; Message from Angel to Wiz.

  :dan!~h@localhost PRIVMSG #coolpeople :Hi everyone!
    ; Message from dan to the channel
    #coolpeople */
void Server::parsePrivmsg(Client &client)
{
    // ERR_NOSUCHNICK (401) no ha encontrado el nick
    // ERR_CANNOTSENDTOCHAN no esta en el canal
    // (404) ERR_NORECIPIENT //no target
    // (411) ERR_NOTEXTTOSEND (412) no text
    try{
        //PRIVMSG TARGET MENSAGAWEAWDAWD
        if(client.getFullmsg().size() == 1)
            throw ERR_NEEDMOREPARAMS;
        if(client.getFullmsg().size() == 2)
        {
            if(client.getFullmsg()[1][0] == ':')
                throw ERR_NORECIPIENT;
            else
                throw ERR_NOTEXTTOSEND;
        }
        //PRIVMSG jorge hola
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
                    response += client.getNick() + " PRIVMSG " + name_vec[i];
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