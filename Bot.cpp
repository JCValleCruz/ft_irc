/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sbenitez <sbenitez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 00:21:40 by sbenitez          #+#    #+#             */
/*   Updated: 2026/03/22 17:06:23 by sbenitez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"

// Utils

bool Bot::isCommand(const std::string &msg)
{
	if (msg.empty() || msg[0] != '!')
		return false;
	return true;
}

std::string Bot::getCommand(const std::string &msg)
{
	if (!isCommand(msg))
		return "";
	size_t spacePos = msg.find(' ');
	if (spacePos == std::string::npos)
		return msg.substr(1);
	return msg.substr(1, spacePos - 1);
}

std::string Bot::getChannelFromMessage(Client &client)
{
	std::vector<std::string> fullmsg = client.getFullmsg();
	if (fullmsg.size() >= 2 && fullmsg[1][0] == '#')
		return fullmsg[1];
	return "";
}

void Bot::sendToChannel(Server &server, const std::string &channelName, const std::string &message)
{
	int chanIndex = server.findChannelNumber(channelName);
	if (chanIndex == -1)
		return;
	Channel &channel = server.channels[chanIndex];
	std::string response = ":GabiBot!bot@" + server.gethostName() + " PRIVMSG " + channelName + " :" + message + "\r\n";
	
	std::vector<Client> users = channel.getClients();
	for (size_t i = 0; i < users.size(); i++)
	{
		if (users[i].getSocket() != -1) // Dont send bot (socket -1)
			send(users[i].getSocket(), response.c_str(), response.length(), 0);
	}
}

// Main command handler

void Bot::handleCommand(Client &client, Server &server)
{
	std::vector<std::string> fullmsg = client.getFullmsg();
	
	if (fullmsg.size() < 3 || fullmsg[2].empty() || fullmsg[2][0] != ':')
		return;

	std::string message = fullmsg[2].substr(1); // Remove leading ':'
	for (size_t i = 3; i < fullmsg.size(); i++)
		message += " " + fullmsg[i];
	
	if (!isCommand(message))
		return;
	
	std::string cmd = getCommand(message);
	std::string channelName = getChannelFromMessage(client);

	if (channelName.empty())
		return;

	if (cmd == "help")
		cmd_help(client, server, channelName);
	else if (cmd == "ping")
		cmd_ping(client, server, channelName);
	else if (cmd == "info")
		cmd_info(client, server, channelName);
	else if (cmd == "users")
		cmd_users(client, server, channelName);
}
// Commands

void Bot::cmd_help(Client &client, Server &server, const std::string &channelName)
{
	(void)client;

	sendToChannel(server, channelName, "|=== Bot command list ===|");
	sendToChannel(server, channelName, "!help - Show this help message");
	sendToChannel(server, channelName, "!ping - Test bot responsiveness");
	sendToChannel(server, channelName, "!info - Get channel information");
	sendToChannel(server, channelName, "!users - List users in the channel");
}

void Bot::cmd_ping(Client &client, Server &server, const std::string &channelName)
{
	(void)client;

	sendToChannel(server, channelName, "Amoh a echanoh un pinpon, loko!");
}

void Bot::cmd_info(Client &client, Server &server, const std::string &channelName)
{
	(void)client;
	
	int chanIndex = server.findChannelNumber(channelName);
	if (chanIndex == -1)
	{
		sendToChannel(server, channelName, "Error: Channel not found.");
		return;
	}
	Channel &channel = server.channels[chanIndex];
	std::ostringstream oss;

	oss << "Users: " << channel.getClients().size();
	sendToChannel(server, channelName, oss.str());

	oss.str("");
	oss << "Moderators: " << channel.getModerators().size();
	sendToChannel(server, channelName, oss.str());

	std::string topic = channel.getTopic();
	if (!topic.empty())
		sendToChannel(server, channelName, "Topic: " + topic);
	else
		sendToChannel(server, channelName, "No topic set.");

	std::string modes = channel.getModes();
	if (!modes.empty())
		sendToChannel(server, channelName, "Modes: " + modes);
}

void Bot::cmd_users(Client &client, Server &server, const std::string &channelName)
{
	(void)client;

	int chanIndex = server.findChannelNumber(channelName);
	if (chanIndex == -1)
	{
		sendToChannel(server, channelName, "Error: Channel not found.");
		return;
	}

	Channel &channel = server.channels[chanIndex];
	std::vector<Client> users = channel.getClients();
	std::vector<Client> mods = channel.getModerators();

	std::ostringstream oss;
	oss << "*** Users (" << users.size() << ") ***";
	sendToChannel(server, channelName, oss.str());

	for (size_t i = 0; i < users.size(); i++)
	{
		std::string prefix = "";
		for (size_t j = 0; j < mods.size(); j++)
		{
			if (mods[j].getNick() == users[i].getNick())
			{
				prefix = "@";
				break;
			}
		}
		sendToChannel(server, channelName, prefix + users[i].getNick());
	}
}