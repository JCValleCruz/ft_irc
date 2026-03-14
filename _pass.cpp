/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _pass.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 13:28:46 by jose-rig          #+#    #+#             */
/*   Updated: 2026/03/14 12:56:44 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::parsePass(Client &client){
	std::vector<std::string> fullmsg = client.getFullmsg();
	std::cout << PURPLE << fullmsg.size() << std::endl;
	std::cout << WHITE;
	if(client.getVerify() == true)
	{
		err(ERR_ALREADYREGISTERED, this->hostname, client);
		return ;
	}
	else if(fullmsg.size() != 2)
	{
		err(ERR_NEEDMOREPARAMS, this->hostname, client);
		return;
	}
	else if(fullmsg[1] == this->password)
	{
		std::cout << "Correct Password(" << client.getSocket() << ")" << std::endl;
		client.setVerify(true);
	}
	else if(client.getRetries() > 0 && fullmsg[1] != this->password)
	{
		err(ERR_PASSWDMISMATCH, this->hostname, client);
		client.decreaseRetries();
		if(client.getRetries() == 0)
		{
			client.sendResponse();
			client.setResponse("No more retries. Bye bye");client.sendResponse();
			this->disconnectClient(client);
		} 
		return;
	}		
}