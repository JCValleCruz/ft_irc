/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _join.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jormoral <jormoral@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:47:44 by jormoral          #+#    #+#             */
/*   Updated: 2025/09/22 12:47:47 by jormoral         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::joinChannel(Client &client)
{
	try{// JOIN #test,#prueba 1234,5454     -> test(1234) prueba(5454)
		std::vector<std::string> fullmsg = client.getFullmsg();
		std::vector<std::string> key;
		std::vector<std::string> names;
		if(fullmsg.size() <= 1) 
			throw(ERR_NEEDMOREPARAMS);			//split for ',' para unirse a varios. nasty
		if(fullmsg.size() > 1)
			names = ft_split(fullmsg[1], ',', '\0');
		if(fullmsg.size() > 2) //size empieza en 1. yaaaa
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
				for(size_t x = 0; x < names.size(); x++) //// JOIN #test,#prueba 1234,5454     -> test(1234) prueba(5454)     --- JOIN #test,#prueba 1234
				{																		// names["test", "prueba"] key["1234", "5454"]
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
									throw ERR_NEEDMOREPARAMS; // ok 
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