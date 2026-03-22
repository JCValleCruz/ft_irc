/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sbenitez <sbenitez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:49:46 by jormoral          #+#    #+#             */
/*   Updated: 2026/03/22 16:23:38 by sbenitez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <vector>

template <typename T> void printVector(T &array)
{
	typename T::iterator it = array.begin();
	typename T::iterator ite = array.end();
	int i = 0;
	std::cout << GREEN <<"Printing Vector:" << std::endl;
	while(it != ite)
	{
		std::cout << "[" << i << "]"<< (*it) << "\n";
		it++;
		i++;
	}
	std::cout << WHITE << std::endl;
}


Server::Server(int port, char *password){
    this->port = port;
    this->password = password;
	
    this->server_socket = initServerSocket();
    this->server_address = initServerAddress(port);
	
    bind(this->server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_socket, 6);
    initPolls();
    initHostName();
	initBotClient();
    printServer();
}

int Server::initServerSocket(){
    int servsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(servsocket == -1)
		errorPrint("Failed to create socket");
    int one = 1;
    setsockopt(servsocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    fcntl(servsocket, F_SETFL, O_NONBLOCK);
	
    return(servsocket);
}

sockaddr_in Server::initServerAddress(int port){
    sockaddr_in address;
	memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    return (address);
}

void Server::initPolls()
{
    struct pollfd paul;
    paul.fd = this->server_socket;
    paul.events = POLLIN;
    paul.revents = 0;
    this->polls.push_back(paul);
}

void Server::initHostName(){
    char aux[1024];
    if(gethostname(aux, 1024) == -1)
        errorPrint("Unable to fetch hostname");
    this->hostname = aux;
}

int Server::checkConnections(void) {
    int result = poll(&this->polls[0], polls.size(), -1);
    if (result == -1 && !g_signal) 
		errorPrint("Error while trying to poll()");
    return (result);
}

int Server::updateConnections()
{
	unsigned int i = 0;
	while(i < this->polls.size())
	{
		if(this->polls[i].revents > 0)
		{
			if(this->polls[i].fd == this->server_socket)
				newClient();
			else
				manageClientMessage(this->clients[this->polls[i].fd]);
		}
		i++;
	}

	return(0);
}

void Server::manageClientMessage(Client &client)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	ssize_t len = recv(client.getSocket(), buffer, sizeof(buffer) - 1, 0); // sizeof buffer - 1
	if(len == -1)
	{
		std::cerr << "Recv error" << std::endl;
		exit(1);
	}
	if(len == 0)
	{
		this->parseQuit(client);
	}
	if (len > 0)
	{
		std::string accumulated = client.getMessage() + std::string(buffer);
		size_t pos;
		while ((pos = accumulated.find('\n')) != std::string::npos)
		{
			client.setMessage(accumulated.substr(0, pos + 1));
			parseMessage(client);
			accumulated = accumulated.substr(pos + 1);
		}
		client.setMessage(accumulated);
	}
}

void Server::parseMessage(Client &client)
{
	std::string message(client.getMessage());
	if(!message.empty() && message[message.size() - 1] == '\n')
		message.erase(message.size() - 1);	
	if(!message.empty() && message[message.size() - 1] == '\r')
		message.erase(message.size() - 1);
	if(message.size() == 0)
		return;

	client.setMessage(message);

	client.setFullmsg(ft_split(message , ' ', ':'));
	std::vector<std::string> fullmsg = client.getFullmsg();

	printVector(client.getFullmsg());

	std::string system_commands[5] = {"PASS", "NICK", "USER", "CAP", "QUIT"};
	for(int i = 0; i < 5; i++)
	{
		if(fullmsg[0] == system_commands[i])
		{
			system_switch(i, client);
			client.sendResponse();
		}
		if(client.getNick() != "" && client.getUsername() != "")
			client.setHostname();
	}
	if(client.getUserVerified() == false)                 //MODE a secas? comprobar comandos con y sin arguments
		return;                                                                                   //+limit +invite +topic +key +o=moderator y con el '-'
	std::string user_commands[9] = {"JOIN", "KICK", "NICK", "PART", "PRIVMSG", "MODE", "TOPIC", "INVITE", "NOTICE"};
	for(int i = 0; i < 9; i++)                                                // TOPIC #canal? -> le dice el topic a la persona? de todos los canales?
	{
		if(fullmsg[0] == user_commands[i])
		{
			user_switch(i, client);
			client.sendResponse();
			return;
		}
	}
}

void Server::user_switch(int i, Client &client)
{
	switch(i)
	{
		case 0:
			joinChannel(client);break;
		case 1:
			parseKick(client);break;
		case 2:
			parseChangeNick(client);break;
		case 3:
			parsePart(client);break;
		case 4:
			parsePrivmsg(client);break;
		case 5:
			parseMode(client);break;
		case 6:
			parseTopic(client);break;
		case 7:
			parseInvite(client);break;
		case 8:
			parseNotice(client);break;
	}
}

void Server::system_switch(int i, Client &client)
{
	switch(i)
	{
		case 0:
			parsePass(client);break;
		case 1: if(client.getNick() == "" )parseNick(client);break;
		case 2:
			parseUser(client);break;
		case 3:
			parseCap(client);break;
		case 4:
			parseQuit(client);
	}
}

void Server::newClient()
{
	sockaddr_in address;
	socklen_t address_size = sizeof(address);
	int clisocket = accept(this->server_socket, (sockaddr*)&address, &address_size);
	if(clisocket == -1)
		errorPrint("Failed to create new Client Socket");
	if(fcntl(clisocket, F_SETFL, O_NONBLOCK) == -1)
		errorPrint("Failed to set new Client socket options");
	Client createdClient(clisocket, address);
	struct pollfd createdPoll;
	createdPoll.fd = clisocket;
	createdPoll.events = POLLIN;
	createdPoll.revents = 0;
	this->polls.push_back(createdPoll);
	this->clients[clisocket] = createdClient;
	std::cout << "Client socket " << clisocket << " connected."<< std::endl;
}

void Server::disconnectClient(Client &client){
	Client aux = client;
	for(std::vector<struct pollfd>::iterator it = this->polls.begin(); it != this->polls.end(); ++it)
	{
		if(aux.getSocket() == (*it).fd)
		{
			this->polls.erase(it);
			close(aux.getSocket());
			break;
		}
	}
}

std::string Server::gethostName(){
	return(this->hostname);
}


void Server::notifyAll(std::string response){
	response += "\n\r";
	for(std::map<int, Client>::iterator  it = this->clients.begin(); it != this->clients.end(); it++)
	{
		send((*it).second.getSocket(), response.c_str(), response.length(), AF_INET);
	}
	std::cout << "server::Notify =" << response << std::endl;

}

// Bot Management

Server::~Server()
{
	delete botClient;
}

void Server::initBotClient()
{
	sockaddr_in fake_addr;
	memset(&fake_addr, 0, sizeof(fake_addr));

	botClient = new Client(-1, fake_addr);
	botClient->setNick("GabiBot");
	botClient->setUsername("bot");
	botClient->setVerify(true);

	std::string botHostname = "GabiBot!bot@" + this->hostname;
	botClient->setHostnameManual(botHostname);
	
	// We cant use setHostname because it needs DNS lookup
}

Client* Server::getBotClient()
{
	return botClient;
}

void Server::checkBotShouldLeave(Channel &channel)
{
	std::vector<Client> users = channel.getClients();

	// If the bot is the only one left, leave the channel
	if (users.size() == 0 || (users.size() == 1 && users[0].getNick() == botClient->getNick()))
	{
		std::string response = ":GabiBot!bot@" + this->hostname + " PART " + channel.getName() + "\r\n";
		
		for (size_t i = 0; i < users.size(); i++)
		{
			if (users[i].getSocket() != -1) // Dont send bot (socket -1)
				send(users[i].getSocket(), response.c_str(), response.length(), 0);
		}
		
		channel.removeClient(*botClient);
	}
}
