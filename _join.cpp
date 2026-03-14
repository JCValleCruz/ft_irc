/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _join.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:47:44 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/14 12:56:25 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::joinChannel(Client &client)
{
	try{
		std::vector<std::string> fullmsg = client.getFullmsg();
		std::vector<std::string> key;
		std::vector<std::string> names;
		if(fullmsg.size() <= 1)
			throw(ERR_NEEDMOREPARAMS);
		if(fullmsg.size() > 1)
			names = ft_split(fullmsg[1], ',', '\0');
		if(fullmsg.size() > 2)
			key = ft_split(fullmsg[2], ',', '\0');
		for (size_t i = 0; i < names.size(); i++)
		{
			
			if(names[i][0]  != '#')
				throw ERR_UNKNOWNCOMMAND;
			if(!this->channelExists(names[i]))
			{
				Channel temp(names[i], *this);
				temp.addToMods(client);
				temp.addClient(client);
				this->channels.push_back(temp);
				std::cout << "SIZE de canales en el Server: " << this->channels.size() << std::endl;
			}
			else
			{
				int num = this->findChannelNumber(names[i++]);
				if(this->channels[num].isInChannel(client.getNick()))
					throw(ERR_ALREADYINCHAN);
				for(size_t x = 0; x < names.size(); x++)
			{
					for(size_t i = 0; i < this->channels.size(); i++)
					{
						if(this->channels[i].getName() == names[x]) // i =localizamos el canal dentro del vector de canales server
						{
							Channel &chan = this->channels[i];
							if(this->channels[i].hasModeKey() == true) // tiene contraseña 
							{
								if(x + 1 <= key.size() && key[x] == this->channels[i].getKey()) // comprobar la contraseña
								{
									if(chan.gethasLimit() == true && chan.getLimit() > chan.getClients().size())
									{
										if(chan.hasInviteOnly() == true && chan.isInvited(client.getNick())) 
											this->channels[i].addClient(client);
										else if(chan.hasInviteOnly() == false)
											this->channels[i].addClient(client);
										else
											throw(ERR_INVITEONLYCHAN);
									}
									else if (chan.gethasLimit() == true)
										throw(ERR_CHANNELISFULL);
									else if((chan.hasInviteOnly() == false) ||( chan.hasInviteOnly() == true && chan.isInvited(client.getNick())))
											this->channels[i].addClient(client);
									else
										throw(ERR_INVITEONLYCHAN);
								}
								else if(x + 1 > key.size())
									throw ERR_BADCHANNELKEY;
								else
									throw ERR_BADCHANNELKEY; //compila
							}
							else // no tiene contraseña 
							{
								if(chan.gethasLimit() == true && chan.getLimit() > chan.getClients().size())
									{
										if(chan.hasInviteOnly() == true && chan.isInvited(client.getNick())) 
											this->channels[i].addClient(client);
										else if(chan.hasInviteOnly() == false)
											this->channels[i].addClient(client);
										else
											throw(ERR_INVITEONLYCHAN);
									}
									else if (chan.gethasLimit() == true)
										throw(ERR_CHANNELISFULL);
									else if((chan.hasInviteOnly() == false) ||( chan.hasInviteOnly() == true && chan.isInvited(client.getNick())))
											this->channels[i].addClient(client);
									else
										throw(ERR_INVITEONLYCHAN);
								this->channels[i].addClient(client);
								std::cout << "channel size " << this->channels[i].getClients().size() << std::endl;
							}
						}
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