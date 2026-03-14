/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _nick.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 12:25:07 by jose-rig          #+#    #+#             */
/*   Updated: 2025/09/29 20:02:50 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::parseNick(Client &client){
	try{
		
		if(client.getVerify() == false)
			throw(ERR_NOTREGISTERED);
		if(client.getFullmsg().size() != 2)
			throw(ERR_NONICKNAMEGIVEN);
		if(this->nickInUse(client.getFullmsg()[1]))
			throw(ERR_NICKNAMEINUSE);
		if(nickInvalidChars(client.getFullmsg()[1]) || client.getFullmsg().size() > 2) 
			throw(ERR_ERRONEUSNICKNAME);
		
		client.setNick(client.getFullmsg()[1]);
		client.changeNick = true;

		std::cout << "Nick changed to:" << client.getNick() << std::endl;
	}
	catch(ERR num)
	{
		err(num, this->hostname, client);
	}

}

void Server::parseChangeNick(Client &client)				//Yo hubiera usado esto como pasarela siempre comprobando FLAG
{
	parseNick(client);
	if(client.changeNick == true)
	{
		std::string response = client.getHostname() + " changed nick to: " + client.getFullmsg()[1] + "\r\n";
		this->notifyAll(response);
	}
	client.changeNick = false;
}

int Server::nickInvalidChars(std::string nick)
{
	char first = nick[0];
	if(first == '#' || first == ':' || first == ' ')
		return 1;
	if(first >= '0' && first <= '9')
		return (1);
	int i = 0;
	while(nick[i])
	{
		if(!isalnum(nick[i]) && nick[i] != '[' && nick[i] != ']'
		&& nick[i] != '{' && nick[i] != '}' && nick[i] != '\'' && nick[i] != '|')
			return 1;
		i++;
	}
	return 0;
}

int Server::nickInUse(std::string nick)
{
	std::map<int, Client >::iterator it = this->clients.begin();
	std::map<int, Client >::iterator ite = this->clients.end();
	while(it != ite)
	{
		Client temp = (*it).second;
		if(temp.getNick() == nick)
			return 1;
		it++;
	}
	return(0);
}

