/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:50:24 by jormoral          #+#    #+#             */
/*   Updated: 2025/09/29 19:28:21 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int socket, sockaddr_in address){
	this->socket = socket;
	this->address = address;
	this->a_len = sizeof(address);
	this->nick = "";
	this->user = "";
	this->realname = "";
	this->message = "";
	this->verified = false;
	this->username_verified = false;
	this->argument = "";
	this->retries = 3;
	if(getpeername(this->socket, (struct sockaddr*)&this->address, &this->a_len) == -1)
		std::cerr << "Unable to get hostname." << std::endl;
	this->host = gethostbyaddr(&this->address.sin_addr, sizeof(this->address.sin_addr), AF_INET);
	this->changeNick = false;
}

int Client::getSocket(){
	return this->socket;
}
void Client::printMessage(){
	std::cout << BLUE;
	std::cout << "\"" << this->message << "\"" << std::endl;
	std::cout << WHITE;
};

Client::Client(){};
Client::~Client(){};


std::string Client::getMessage(){return (this->message);}
void Client::setMessage(std::string message) { this->message = message;}
void Client::setSocket(int socket){this->socket = socket;}
bool Client::getVerify(){return(this->verified);};
void Client::setVerify(bool veri){this->verified = veri;};
std::string Client::getArgument(){return (this->argument);}
void Client::setArgument(std::string argument) { this->argument= argument;}
std::string Client::getNick(){return (this->nick);}
void Client::setNick(std::string nick) { this->nick = nick;}

void Client::decreaseRetries()
{
	--this->retries;
	std::cout << "Tries remaining: " << this->retries << std::endl;
}
int Client::getRetries(){
	return(this->retries);
}

bool Client::getUserVerified(){
	return this->username_verified; 
}

std::string Client::getUsername(){return this->user;};

void		Client::setUsername(std::string user){
	this->user = user;
	this->username_verified = true;};

std::string Client::getResponse(){return (this->response);}

void Client::setResponse(std::string response) {
	this->response = response;}
	
void Client::sendResponse(){
	std::cout << GREEN << ":"<<this->response << WHITE << std::endl;
	if(this->response == "")
		return;
	this->response += "\n\r";
	send(this->socket, this->getResponse().c_str(), this->getResponse().length(), AF_INET);
	std::cout << "Client::Response sent=" << this->getResponse() << std::endl;
	this->setResponse("");
};


std::string Client::getHostname(){
	return this->hostname;
}


void Client::setHostname()
{
	this->hostname = this->nick + "!" + this->user + "@" + this->host->h_name;
}

std::vector<std::string> Client::getFullmsg(){
	return(this->fullmsg);
};

void Client::setFullmsg(std::vector<std::string> fullmsg){
	this->fullmsg = fullmsg;
};
