/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _quit.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jormoral <jormoral@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:15:38 by jormoral          #+#    #+#             */
/*   Updated: 2025/09/18 12:10:10 by jormoral         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//si este client es el unico moderador y hay otra persona le damos moderador a el
void Server::nextModerator(std::string nick, Channel &channel)
{
	if(channel.getClients().size() > 1)
	{
		if(channel.getModerators().size() == 1 && channel.isAMod(nick))
		{
			size_t i = 0;
			while (i < channel.getClients().size())
			{
				if(channel.getClients()[i].getNick() != nick)
				{
					channel.addToMods(channel.getClients()[i]);
					channel.removeFromMods(nick);
					return ;
				}
				i++;
			}
		}
	}
}

void Server::parseQuit(Client &client)
{
	std::string response = ":" + client.getHostname() + " PART ";
	for(size_t i = 0; i < this->channels.size(); i++)
	{
		if(this->channels[i].isInChannel(client.getNick()) == 1)
		{
			std::string nick = client.getNick();
			nextModerator(nick ,this->channels[i]);
			this->channels[i].removeClient(client);
			if(this->channels[i].getClients().size() == 0)
				deleteChannel(this->channels[i].getName());
			else
				this->channels[i].sendResponseChannel(response + this->channels[i].getName(), client, 0);
		}
	}
	disconnectClient(client);
}

void Server::deleteChannel(std::string channel)
{
	for(std::vector<Channel>::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		if((*it).getName() == channel)
		{
			this->channels.erase(it);
			return;
		}
	}
}
