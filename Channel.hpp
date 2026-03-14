/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:57:12 by jvalle-d          #+#    #+#             */
/*   Updated: 2026/03/14 12:57:13 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Server.hpp"
#include "irc.hpp"

class Server;
class Client;

class Channel{
private:
	std::string name;
	std::string response;
	
	bool modeLimit;
	size_t maxsize;

	bool modeKey;
	std::string key;

	bool modeInvite;

	bool modeTopic;
	std::string topic;

	std::vector<Client> moderators;
	std::vector<Client> clients;

	Server *server;
public:
	Channel(std::string name, Server &server);
	~Channel();

	int		addClient(Client &client);
	void	addToMods(Client &client);
	void	removeFromMods(std::string nick);
	void	removeFromMods(Client &client);
	int		isAMod(std::string name);
	int		isInChannel(std::string name);

	void	setName(std::string name);
	std::string getName();
	bool	gethasLimit();
	void	setHasLimit(bool);
	void	setLimit(size_t maxsize);
	size_t		getLimit();
	bool	hasModeKey();
	void	setModeKey(bool);
	void	setKey(std::string key);
	std::string getKey();
	bool	hasInviteOnly();
	void	setInviteOnly(bool);
	std::vector<std::string> invitedClients;
	bool	isInvited(std::string tar);
	void setTopic(std::string topic);
	std::string getTopic(void);
	void setModeTopic(bool);
	bool getModeTopic(void);
	void	setResponse(std::string responset);
	std::string getResponse();
	void	sendResponseChannel(std::string response, Client &client, int privflag);
	void removeClient(Client &client);
	std::vector<Client> getClients();
	std::vector<Client> getModerators(){return this->moderators;};
	int kickClient(std::string cliname, Client &mod, std::string reason);
	int clientLevelInChannel(std::string name);
	std::string getModes();
};

#endif