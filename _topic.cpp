/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _topic.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:50:32 by jormoral          #+#    #+#             */
/*   Updated: 2025/10/08 19:23:20 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void rplTopic(Client &client, Channel &chan)
{
	std::string response;
    if(chan.getTopic() == "")
    {
        response = ":" +  client.getHostname() + " 331 " + client.getNick() + " " + chan.getName() + " :No topic is set";
        client.setResponse(client.getResponse() + response);
    }
    else
    {
        response = ":" + client.getHostname() + " 332 " + client.getNick() + " " + chan.getName() + " :" + chan.getTopic();
        client.setResponse(client.getResponse() + response);
    }
}

void Server::parseTopic(Client &client)
{
    std::string response = "";
    try
    {
        if(client.getFullmsg().size() == 1)
            throw(ERR_NEEDMOREPARAMS);
        if(!channelExists(client.getFullmsg()[1]))
                throw(ERR_NOSUCHCHANNEL);
        int channel_index = findChannelNumber(client.getFullmsg()[1]);
        Channel &temp = this->channels[channel_index];
        if(client.getFullmsg().size() == 2)
        {
            if(!temp.isInChannel(client.getNick()))
                throw(ERR_NOTONCHANNEL);
            rplTopic(client, temp);
            client.sendResponse();
        }
        else if(client.getFullmsg().size() == 3)
        {
			if(!temp.isInChannel(client.getNick()))
				throw(ERR_NOTONCHANNEL);
            if(temp.getModeTopic() == true)
            {
                if(!temp.isAMod(client.getNick()))
                    throw(ERR_CHANOPRIVSNEEDED);
            } 
            if(client.getFullmsg()[2] == ":") //clean topic
            {
                temp.setTopic("");
                response = ":" + client.getHostname() + " TOPIC " + temp.getName() + " :";
            }
            else if(client.getFullmsg()[2].length() > 390) //truncate
            {
                temp.setTopic(client.getFullmsg()[2].erase(0, 1).substr(0, 389));
                response = ":" + client.getHostname() + " TOPIC " + temp.getName() + " :" + temp.getTopic();
            }
            else if(client.getFullmsg()[2][0] == ':') // all good
            {
                temp.setTopic(client.getFullmsg()[2].erase(0, 1));
                response = ":" + client.getHostname() + " TOPIC " + temp.getName() + " :" + temp.getTopic();
            }
            else
                throw(ERR_UNKNOWN);
            temp.setResponse(response);
            temp.sendResponseChannel(response, client, 0);
        }
        else
            throw(ERR_UNKNOWN);
    }
    catch(ERR num)
    {
        err(num, this->hostname, client);
    }
}
