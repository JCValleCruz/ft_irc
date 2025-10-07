/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:50:05 by jormoral          #+#    #+#             */
/*   Updated: 2025/10/07 19:58:05 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"
#include "Server.hpp"

int g_signal = 0;

void signalHandler(int signum) { g_signal = signum; }

std::map<ERR, std::string> errorMessages;
void initializeErrorMessages() {
    errorMessages[ERR_UNKNOWNERROR] = "Could not process multiple invalid parameters";
    errorMessages[ERR_NOSUCHNICK] = "No such nick/channel";
    errorMessages[ERR_NOSUCHSERVER] = "No such server";
    errorMessages[ERR_NOSUCHCHANNEL] = "No such channel";
    errorMessages[ERR_CANNOTSENDTOCHAN] = "Cannot send to channel";
    errorMessages[ERR_TOOMANYCHANNELS] = "You have joined too many channels";
    errorMessages[ERR_WASNOSUCHNICK] = "There was no such nickname";
    errorMessages[ERR_TOOMANYTARGETS] = "Duplicate recipients. No message delivered";
    errorMessages[ERR_NOORIGIN] = "No origin specified";
    errorMessages[ERR_NORECIPIENT] = "No recipient given (<command>)";
    errorMessages[ERR_NOTEXTTOSEND] = "No text to send";
    errorMessages[ERR_NOTOPLEVEL] = "No toplevel domain specified";
    errorMessages[ERR_WILDTOPLEVEL] = "Wildcard in toplevel domain";
    errorMessages[ERR_BADMASK] = "Bad Server/host mask";

    errorMessages[ERR_UNKNOWNCOMMAND] = "Unknown command";
    errorMessages[ERR_NOMOTD] = "MOTD File is missing";
    errorMessages[ERR_NOADMININFO] = "No administrative info available";
    errorMessages[ERR_FILEERROR] = "File error";

    errorMessages[ERR_NONICKNAMEGIVEN] = "No nickname given";
    errorMessages[ERR_ERRONEUSNICKNAME] = "Erroneous nickname";
    errorMessages[ERR_NICKNAMEINUSE] = "Nickname is already in use";
    errorMessages[ERR_NICKCOLLISION] = "Nickname collision KILL";
    errorMessages[ERR_UNAVAILRESOURCE] = "Nick/channel is temporarily unavailable";

    errorMessages[ERR_USERNOTINCHANNEL] = "They aren't on that channel";
    errorMessages[ERR_NOTONCHANNEL] = "You're not on that channel";
    errorMessages[ERR_USERONCHANNEL] = "is already on channel";
    errorMessages[ERR_NOLOGIN] = "User not logged in";
    errorMessages[ERR_SUMMONDISABLED] = "SUMMON has been disabled";
    errorMessages[ERR_USERSDISABLED] = "USERS has been disabled";

    errorMessages[ERR_NOTREGISTERED] = "You have not registered";
    errorMessages[ERR_NEEDMOREPARAMS] = "Not enough parameters";
    errorMessages[ERR_ALREADYREGISTERED] = "You may not reregister";
    errorMessages[ERR_PASSWDMISMATCH] = "Password incorrect";
    errorMessages[ERR_YOUREBANNEDCREEP] = "You are banned from this server";
    errorMessages[ERR_YOUWILLBEBANNED] = "You will be banned";
    errorMessages[ERR_KEYSET] = "Channel key already set";
    errorMessages[ERR_CHANNELISFULL] = "Cannot join channel (+l)";
    errorMessages[ERR_UNKNOWNMODE] = "Unknown mode character";
    errorMessages[ERR_INVITEONLYCHAN] = "Cannot join channel (+i)";
    errorMessages[ERR_BANNEDFROMCHAN] = "Cannot join channel (+b)";
    errorMessages[ERR_BADCHANNELKEY] = "Cannot join channel (+k)";
    errorMessages[ERR_BADCHANMASK] = "Bad Channel Mask";
    errorMessages[ERR_NOCHANMODES] = "Channel doesn't support modes";
    errorMessages[ERR_BANLISTFULL] = "Ban list is full";

    errorMessages[ERR_NOPRIVILEGES] = "Permission Denied- You're not an IRC operator";
    errorMessages[ERR_CHANOPRIVSNEEDED] = "You're not channel operator";
    errorMessages[ERR_CANTKILLSERVER] = "You can't kill a server!";
    errorMessages[ERR_RESTRICTED] = "Your connection is restricted!";
    errorMessages[ERR_UNIQOPRIVSNEEDED] = "You're not the original channel operator";
    errorMessages[ERR_NOOPERHOST] = "No O-lines for your host";

    errorMessages[ERR_UMODEUNKNOWNFLAG] = "Unknown MODE flag";
    errorMessages[ERR_USERSDONTMATCH] = "Cannot change mode for other users";

    // Additional error codes
    errorMessages[ERR_HELPNOTFOUND] = "Help not found";
    errorMessages[ERR_INVALIDCAPCMD] = "Invalid CAP command";
    errorMessages[ERR_NICKLOCKED] = "You must use a registered nick to perform this command";
    errorMessages[ERR_SASLFAIL] = "SASL authentication failed";
    errorMessages[ERR_SASLTOOLONG] = "SASL message too long";
    errorMessages[ERR_SASLABORTED] = "SASL authentication aborted";
    errorMessages[ERR_SASLALREADY] = "You have already authenticated using SASL";

    errorMessages[ERR_INVALIDMODEPARAM] = "Problem with a mode parameter";
    errorMessages[ERR_UNKNOWN] = "Unknown error";
    errorMessages[ERR_USERLEN] = "Username too long";
	errorMessages[ERR_NOSELFKICK] = "Moderators can't kick themselves from channels";
    errorMessages[ERR_ALREADYINCHAN] = "Client is already in the channel";
};

void err(ERR code, std::string hostname, Client& client, std::string target)
{
	std::ostringstream ss;
	ss << code;
	std::string response = ":" + hostname + " " + ss.str() + " " + client.getNick();
    if (!target.empty())
        response += " " + target;
    response += " :" + errorMessages[code];
    client.setResponse(client.getResponse() + response);
}


int main(int argc , char **argv) {
	if(argc != 3)
	{
		std::cerr << "Error Nº Parameters" << std::endl;
		return(1);
	}
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    if((std::atoi(argv[1]) <= 0) || (std::atoi(argv[1]) > 65535)){							// ni letas ni negativos
		std::cerr << "Port was incorrect, please input a valid port number"	<< std::endl;
		return (1);
	}
	Server server(std::atoi(argv[1]), argv[2]);
	initializeErrorMessages();
    while(!g_signal)
	{
		if(server.checkConnections() > 0)
			server.updateConnections();
	}
    return 0;
}