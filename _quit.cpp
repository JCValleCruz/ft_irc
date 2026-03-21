/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _quit.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aehrl <aehrl@student.42malaga.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:15:38 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/21 20:36:54 by aehrl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include <vector>

void Server::nextModerator(std::string nick, Channel &channel)
{
	if(!channel.getClients().empty())
	{
			channel.addToMods(channel.getClients()[0]);
			channel.removeFromMods(nick);
			std::string new_mod_nick = channel.getClients()[0].getNick();
			std::string mode_response = ":" + this->hostname + " MODE " + channel.getName() + " +o " + new_mod_nick;
			channel.sendResponseChannel(mode_response, channel.getClients()[0], 0);
			
	}
}

void Server::parseQuit(Client &client)
{
	std::string response = ":" + client.getHostname() + " PART ";
	for(size_t i = 0; i < this->channels.size(); i++)
	{
		if(this->channels[i].isInChannel(client.getNick()) == 1)
		{
			this->channels[i].removeClient(client);
			if (this->channels[i].getModerators().size() == 1 && this->channels[i].isAMod(client.getNick()))
				nextModerator(client.getNick(), this->channels[i]);
			else
				this->channels[i].removeFromMods(client);
			if(this->channels[i].getClients().empty())
				deleteChannel(this->channels[i].getName());
			else{
				this->channels[i].sendResponseChannel(response + this->channels[i].getName(), client, 0);
			}
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
