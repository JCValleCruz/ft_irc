/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sbenitez <sbenitez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 00:21:33 by sbenitez          #+#    #+#             */
/*   Updated: 2026/03/22 01:12:51 by sbenitez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
#define BOT_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "irc.hpp"
#include <ctime>
#include <sstream>

class Server;
class Client;
class Channel;

namespace Bot
{
	void handleCommand(Client &client, Server &server);

	void cmd_help(Client &client, Server &server, const std::string &channelName);
	void cmd_ping(Client &client, Server &server, const std::string &channelName);
	void cmd_info(Client &client, Server &server, const std::string &channelName);
	void cmd_users(Client &client, Server &server, const std::string &channelName);

	bool isCommand(const std::string &msg);
	std::string getCommand(const std::string &msg);
	std::string getChannelFromMessage(Client &client);
	void sendToChannel(Server &server, const std::string &channelName, const std::string &message);
}

#endif