/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_Utils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jormoral <jormoral@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:49:55 by jormoral          #+#    #+#             */
/*   Updated: 2025/09/22 12:49:56 by jormoral         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"

int	Server::getClientSocket(std::string name){
	std::map<int, Client>::iterator it = this->clients.begin();
	while(it != this->clients.end())
	{
		if(name == (*it).second.getNick())
			return((*it).first);
		++it;
	}
	return 0;
}


int Server::count_char(std::string message, char delim)
{
	int res = 0;
	for(int i = 0; message[i]; i++)
	{
		if(message[i] == delim)
			res++;
	}
	return (res);
}

void Server::printVector(std::vector<std::string> vec)
{
	for(unsigned long i = 0; i < vec.size(); i++)
		std::cout << "Pos in vector: " << i << " = "<< vec[i] << std::endl;
	
}

void errorPrint(const std::string message){
    std::cerr << message << std::endl;
};
void Server::printServer()
{
    std::cout << "Password " << this->password << std::endl;
    std::cout << "Port " << this->port << std::endl;
    std::cout << "Server Socket " << this->server_socket<< std::endl;
    std::cout << "Host Name " << this->hostname << std::endl;
}
int Server::findChannelNumber(std::string channelname)
{
	size_t i = 0;
	while(i < this->channels.size())
	{
		if(this->channels[i].getName() == channelname)
			return i;
		i++;
	}
	return -1;
}


int	Server::channelExists(std::string chaname)
{
	for(size_t i = 0; i < this->channels.size(); i++)
	{
		if(this->channels[i].getName() == chaname)
			return 1;
	}
	return 0;
}

std::vector<std::string> ft_split(std::string str, char skip, char stop)
{
	std::vector<std::string> split;
	int i = 0;
	std::string extract;
	size_t skpos = 0;
	size_t stopnum = 0;
	while(str[i] && skpos != std::string::npos)
	{
		i = str.find_first_not_of(skip, i);
		skpos = str.find(skip, i);
		extract = str.substr(i, skpos - i);
		stopnum = extract.find(stop, 0);
		if (stopnum != std::string::npos)
		{
			extract = extract.substr(0, stopnum);
			if(extract.length() > 0)
				split.push_back(extract);
			std::string remaining = str.substr(str.find_first_of(stop));
			split.push_back(remaining);
			return split;
		}
		split.push_back(extract);
		i = skpos + 1;
	}
	return(split);
}

Channel Server::getChannel(std::string chaname)
{
	size_t i = 0;
	while(i < this->channels.size())
	{
		if(this->channels[i].getName() == chaname)
			break;
	}
	return(this->channels[i]);
}