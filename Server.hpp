/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aehrl <aehrl@student.42malaga.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:57:52 by jvalle-d          #+#    #+#             */
/*   Updated: 2026/03/21 20:43:59 by aehrl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP  
#define SERVER_HPP

#include "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class Client;
class Channel;
class Server{
private:
    std::string password;
    int port;
    int server_socket;
    sockaddr_in server_address;
    std::string hostname;

    int         initServerSocket();
    sockaddr_in initServerAddress(int port);
    void        initPolls();
    void        initHostName();

public:
    Server(int port, char *password);
    ~Server(){};
    std::vector<struct pollfd> polls;
	std::map<int, Client > clients;
	std::vector<Channel > channels;
    int		checkConnections();
	int		updateConnections();
	void	newClient();
	void	manageClientMessage(Client &client);
	void	parseMessage(Client &client);
	void	system_switch(int i, Client &client);
	void	parsePass(Client &client);
	void	parseNick(Client &client);
	int		nickInUse(std::string nick);
	int		nickInvalidChars(std::string nick);
	void	parseUser(Client &client);
	int		userInUse(std::string user);
	void	parseCap(Client &client);
	void	parseQuit(Client &client);
	void	parseKick(Client &client);
	void	parseMode(Client &client);
	void	parseInvite(Client &client);
	void	user_switch(int i, Client &client);
	void	joinChannel(Client &client);
	void	parsePart(Client &client);
	void	parsePrivmsg(Client &client);
	void	parseChangeNick(Client &client);
	void	parseTopic(Client &client);
	void	parseNotice(Client &client);
	int		channelExists(std::string name);
	Channel	getChannel(std::string chaname);
	void	deleteChannel(std::string channel);
	void	disconnectClient(Client &client);
	void	printVector(std::vector<std::string> vec);
    void	printServer();
	int		count_char(std::string message, char delim);
	void	notifyAll(std::string response);
	int		getClientSocket(std::string name);
	std::string	gethostName();
	void	nextModerator(std::string nick, Channel &channel);
	int		findChannelNumber(std::string channelname);
	void setModeInvite(char sign,Channel &temp);
	void setModeTopic(char sign, Channel &temp);
	void setModeKey(char sign, Channel &temp, std::string arg);
	void setModeLimit(char sign, Channel &temp, std::string arg);
	int setModeModerator(char sign, Channel &temp,std::string arg, Client &client);
};

void rplTopic(Client &client, Channel &chan);
void errorPrint(const std::string message);
std::vector<std::string> ft_split(std::string str, char delim, char stop);

#endif