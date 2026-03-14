/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _mode1.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 18:19:58 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/14 12:56:35 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::setModeInvite(char sign, Channel &temp)
{
    if(sign == '+')
        temp.setInviteOnly(true);
    else
        temp.setInviteOnly(false);
}
void Server::setModeTopic(char sign, Channel &temp)
{
    if(sign == '+')
        temp.setModeTopic(true);
    else
        temp.setModeTopic(false);
}
void Server::setModeKey(char sign, Channel &temp, std::string key){
    if(sign == '+')
    {
        for(size_t i = 0; i < key.size(); i++)
        {
           if(!std::isalnum(key[i]))
                throw(ERR_INVALIDMODEPARAM);
        }
        temp.setKey(key);
        temp.setModeKey(true);
    }
    else
        temp.setModeKey(false);
}
void Server::setModeLimit(char sign, Channel &temp, std::string limit){
    for(size_t i = 0 ; i < limit.size(); i++)
    {
        if(limit[i] < '0' || limit[i] > '9')
            throw(ERR_INVALIDMODEPARAM);
    }
    if(sign == '+')
    {
		if(limit.size() > 9 && (std::atol(limit.c_str()) <= 0 || std::atol(limit.c_str()) >= 2147483647))
			throw(ERR_INVALIDMODEPARAM);
		temp.setLimit(std::atol(limit.c_str()));
		temp.setHasLimit(true);
    }
	else
    {
		temp.setHasLimit(false);
    }
}
int Server::setModeModerator(char sign, Channel &chan, std::string name, Client &client){
    if(this->clients.end() == this->clients.find(getClientSocket(name)))
        throw(ERR_NOSUCHNICK);
    std::map<int,Client>::iterator it = this->clients.find(getClientSocket(name)); //probar sin &
    Client &target = (*it).second;
    std::cout << PURPLE <<  target.getNick() << WHITE<< std::endl;
	if(!chan.isInChannel(client.getNick())) //ME
		throw(ERR_NOTONCHANNEL);
	if(!chan.isInChannel(name)) //target
		throw(ERR_USERNOTINCHANNEL);
	if(sign == '+' && chan.isAMod(client.getNick()))
		chan.addToMods(target);
	else if(sign == '-')
	{
		if(chan.getModerators().size() > 1)
			chan.removeFromMods(target);
		else if(chan.getModerators().size() == 1 && name == client.getNick())
			return -1;

	}
	return 1;
}