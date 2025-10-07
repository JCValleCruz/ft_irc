/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jose-rig <jose-rig@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 12:33:49 by jose-rig          #+#    #+#             */
/*   Updated: 2025/09/18 16:49:30 by jose-rig         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string name, Server &server) : server(&server)
{
	std::cout << "Creating Channel:" << name << std::endl;
	this->name = name;
	this->topic = "";
	this->modeKey = false;
	this->key = "";
	this->modeInvite = false;
	this->modeLimit = false;
	this->modeTopic = false;
	this->maxsize = -1;
	this->response = "";
}

int Channel::addClient(Client &client)
{
	//COMPROBAR SI YA ESTA EL CLIENTE EN EL GRUPO

	for(size_t i = 0; i < this->clients.size(); i++)
	{
		if(this->clients[i].getNick() == client.getNick())
		{
			//ERR num??
			std::cout << "Client already in this Channel:" << this->name << std::endl;
			return -1;
		}
	}
	//METER AL CLIENTE EN EL VECTOR DE CLIENTES DEL GRUPO
	this->clients.push_back(client);
	//MANDAR MENSAJE AL GRUPO DE QUE CLIENTE SE HA UNIDO
	std::string response = ":" + client.getHostname() + " JOIN :" + this->name ;
	setResponse(response);
	
	client.setResponse(response + "\r\n");
	//if(this->topic != "")
	rplTopic(client, *this); //rplTopic 332
	client.setResponse(client.getResponse() + "\r\n");
	std::string info_msg = ":" + this->server->gethostName() + " 353 " + client.getNick() + " = " + this->name + " :";
	std::string client_names = "";
	for(size_t i = 0; i < this->clients.size(); i++)
	{
		if(isAMod(this->clients[i].getNick()))
			client_names += "@";

		client_names += this->clients[i].getNick();

		if(i + 1 != this->clients.size())
			client_names += " ";
	}
	info_msg += client_names;
	client.setResponse(client.getResponse() + info_msg + "\r\n"); 
	std::string endofnames = ":" + this->server->gethostName() + " 366 " + client.getNick() + " " + this->name + " :End of /NAMES list.";
	client.setResponse(client.getResponse() + endofnames); client.sendResponse();
	
	sendResponseChannel(this->getResponse(), client, 1);
	//mandar topic rpl
	std::cout << "-debugClient " << client.getNick() << " added to Channel:" << this->name << std::endl;
	return(1);
}

void Channel::removeFromMods(std::string nick)
{
	for(std::vector<Client>::iterator it = this->moderators.begin(); it < this->moderators.end(); ++it)
	{
		if(nick == (*it).getNick())
		{
			this->moderators.erase(it);
			return ;
		}
	}
	return ;
}

int Channel::isAMod(std::string name)
{
	for(size_t i = 0; i < this->moderators.size(); i++)
	{
		if(name == this->moderators[i].getNick())
			return 1;
	}
	return 0;
}


void Channel::addToMods(Client &client)
{
	std::string name = client.getNick();
	for(size_t i = 0; i < this->moderators.size(); i++)
	{
		if(this->moderators[i].getNick() == client.getNick())
		{
			std::cout << "Client already a moderator in this Channel:" << this->name << std::endl;
			return ;
		}
	}
	std::cout << "Client " << client.getNick() << " added as moderator to Channel:" << this->name << std::endl;
	this->moderators.push_back(client);
}
void Channel::removeFromMods(Client &client)
{
	std::string name = client.getNick();
	
	for(std::vector<Client>::iterator it = this->moderators.begin(); it < this->moderators.end(); ++it)
	{
		if((*it).getNick() == client.getNick())
		{
			std::cout << "Client " << client.getNick() << " removed MOD  to Channel:" << this->name << std::endl;
			this->moderators.erase(it);
			return ;
		}
	}
	std::cout << "Client is NOT a moderator in this Channel:" << this->name << std::endl;
	
}

Channel::~Channel(){};

void Channel::setName(std::string name){
	this->name = name;
}
std::string Channel::getName(){
	return(this->name);
}

bool Channel::gethasLimit(){return(this->modeLimit);}

void Channel::setHasLimit(bool truth){
	this->modeLimit = truth;
}

void Channel::setLimit(size_t maxsize){
	this->maxsize = maxsize;
}
size_t Channel::getLimit(){
	return(this->maxsize);
}

bool Channel::hasModeKey(){return(this->modeKey);}
void Channel::setModeKey(bool b){
	this->modeKey = b;
}
void Channel::setKey(std::string key){
	this->key = key;
};
std::string Channel::getKey(){
	return(this->key);
}
bool Channel::hasInviteOnly(){
	return(this->modeInvite);
};
void Channel::setInviteOnly(bool truth){
	this->modeInvite = truth;
}

std::string Channel::getResponse(){return (this->response);}
void Channel::setResponse(std::string response) {
	this->response = response;}
	
void Channel::sendResponseChannel(std::string response, Client &client, int privflag){
	response += "\r\n";
	for(size_t i = 0; i < this->clients.size(); i++)
	{
		if((this->clients[i].getSocket() != client.getSocket()) || privflag == 0)
			send(this->clients[i].getSocket(), response.c_str(), response.length(), AF_INET);
	}
	std::cout << "Channel::Response sent=" << response << std::endl;
	this->setResponse("");
	(void)client;
};

std::vector<Client> Channel::getClients(){
	return this->clients;
}

void Channel::removeClient(Client &client)
{
	for(std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		if(client.getNick() == (*it).getNick())
		{
			this->clients.erase(it);
			return;
		}
	}
	//std::cout << "Channel::removeClient size= " << this->clients.size() << std::endl;

}
int	Channel::isInChannel(std::string name)
{
	for(size_t i = 0; i < this->clients.size(); i++)
	{
		if(name == this->clients[i].getNick())
			return 1;
	}
	return 0;
}
int Channel::kickClient(std::string target, Client &mod, std::string reason)
{

	std::vector<Client>::iterator it = this->clients.begin();
	std::cout << "channel::size before remove " << this->clients.size() << std::endl;

	while(it != this->clients.end())
	{
		// Parameters: <channel> <user> *( "," <user> ) [<comment>]
		// :WiZ!jto@tolsun.oulu.fi KICK #Finnish John
		if(target == (*it).getNick())
		{
			std::string response = ":" + mod.getHostname() + " KICK " + this->getName() + " " + target + reason;
			this->sendResponseChannel(response, mod, 0);
			//(*it).removeChannelfromClient(this);
			this->removeFromMods((*it));
			this->clients.erase(it);
			std::cout << "channel::size after remove " << this->clients.size() << std::endl;
			return 1;
		}
		++it;
	}
	return 0;
}


int Channel::clientLevelInChannel(std::string name){
	int ret = 0;
	size_t i = 0;
	while(i < this->clients.size()) 
	{
		if(name == this->clients[i].getNick())
		{
			ret++;
			if(this->isAMod(name) == 1)
			{
				ret++;
				return(ret);
			}
		}
		i++;
	}
	return (ret);
}

void Channel::setTopic(std::string topic){
	this->topic = topic;
}
std::string Channel::getTopic(void){
	return(this->topic);
}
void Channel::setModeTopic(bool a){
	this->modeTopic = a;
}
bool Channel::getModeTopic(void){
	return(this->modeTopic);
}
//i k t l
std::string Channel::getModes(){
	std::string modes = "+";
	if(this->modeInvite == true)
		modes += "i";
	if(this->modeKey == true)
		modes += "k";
	if(this->modeTopic == true)
		modes += "t";
	if(this->modeLimit == true)
		modes += "l";
	if(modes == "+")
		return ("");
	return modes;
}

bool	Channel::isInvited(std::string tar){
	for(size_t i = 0; i < this->invitedClients.size(); i++)
	{
		std::cout << this->invitedClients[i] << std::endl;
		if(this->invitedClients[i] == tar)
			return true;
	}
	return(false);
}

