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
	
	bool modeLimit; //+l
	size_t maxsize;
	
	
	bool modeKey; //+k
	std::string key;
	
	bool modeInvite; //+i 
	

	
	bool modeTopic;
	std::string topic; //+t
	//std::vector<Client> invited;

	std::vector<Client> moderators; // +o
	std::vector<Client> clients;

	Server *server;
public:
	Channel(std::string name, Server &server); // cambiar topic por password
	~Channel();

	int		addClient(Client &client);
	void	addToMods(Client &client);
	void	removeFromMods(std::string nick);
	void	removeFromMods(Client &client);
	int		isAMod(std::string name);
	int		isInChannel(std::string name);

	//NAME && TOPIC
	void	setName(std::string name);
	std::string getName();
	//LIMIT
	bool	gethasLimit();
	void	setHasLimit(bool);
	void	setLimit(size_t maxsize);
	size_t		getLimit();
	//KEY
	bool	hasModeKey();
	void	setModeKey(bool);
	void	setKey(std::string key);
	std::string getKey();
	//INVITE
	bool	hasInviteOnly();
	void	setInviteOnly(bool);
	std::vector<std::string> invitedClients;
	bool	isInvited(std::string tar);
	//TOPIC 
	void setTopic(std::string topic);
	std::string getTopic(void);
	void setModeTopic(bool);
	bool getModeTopic(void);
	//RESPONSE
	void	setResponse(std::string responset);
	std::string getResponse();
	void	sendResponseChannel(std::string response, Client &client, int privflag);
	//quit
	void removeClient(Client &client);
	std::vector<Client> getClients();
	std::vector<Client> getModerators(){return this->moderators;};
	//kick
	int kickClient(std::string cliname, Client &mod, std::string reason);
	int clientLevelInChannel(std::string name);
	std::string getModes();
};

#endif