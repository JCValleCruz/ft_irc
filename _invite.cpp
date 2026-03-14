/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _invite.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:47:02 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/14 12:56:22 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::parseInvite(Client &client)
{
    try
    {
        if(client.getFullmsg().size() != 3)
            throw(ERR_NEEDMOREPARAMS);
        if(!this->channelExists(client.getFullmsg()[2]))
            throw(ERR_NOSUCHCHANNEL);
        int channel_index = findChannelNumber(client.getFullmsg()[2]);
        Channel &temp = this->channels[channel_index];
        if(!temp.isInChannel(client.getNick()))
            throw(ERR_NOTONCHANNEL);
        if(temp.isInChannel(client.getFullmsg()[1]))
            throw(ERR_USERONCHANNEL);
		std::string response = ":" + client.getHostname() + " INVITE " + client.getFullmsg()[1] + " " + temp.getName();
        temp.setResponse(response);
        send(getClientSocket(client.getFullmsg()[1]), (response + "\r\n").c_str(), response.size() + 2, 0);
		temp.invitedClients.push_back(client.getFullmsg()[1]);
    }
    catch(ERR num)
    {
        err(num, this->hostname, client);
    }
}
