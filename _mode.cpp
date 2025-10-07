/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _mode.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jormoral <jormoral@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:50:49 by jormoral          #+#    #+#             */
/*   Updated: 2025/09/22 18:20:12 by jormoral         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"

//MODE Target String arguments
//Parameters: <target> [<modestring> [<mode arguments>...]]
//                                                                   
//MODE a secas -> (324)"<client> <channel> <modestring> <mode arguments>..."+ (329) creation
//                    Jose-rig #test [+ilk] {: (keys) ?} + hora de creacion - Hexchat?
// MODE #test , existe el canal? perfe throw 324 channelmodeis
// :irc.ejemplo.com 324 tu_nick #test +ntl 50
// A
// B MUST have argument, change setting on a channel (+o)
// C (+k, +l(471)) -> 55/50 o 15/50 MUST arg || (-k -l ) NO ARG
// D MUST not have arguemttn (+i(473), +t(noprivs))
//When the server is done processing the modes,
//a MODE command is sent to all members of the channel containing the mode changes.
//:dan!~h@localhost MODE #foobar -bl+i

void Server::parseMode(Client &client)
{
    try
    {
        std::string response = ":";
        int channel_index = findChannelNumber(client.getFullmsg()[1]);
        if(client.getFullmsg().size() == 1)
            throw ERR_NEEDMOREPARAMS;
        if(channel_index == -1)
            throw ERR_NOSUCHCHANNEL;
        Channel &chan = this->channels[channel_index];
        if(client.getFullmsg().size() == 2)
        {
            response += this->hostname + " 324 " + client.getNick() + " " + chan.getName()  + " " ; 
            if(chan.getTopic() != "")
               response += chan.getTopic() + " ";
            if(chan.getModes().find('l', 0) != std::string::npos)
            {
                std::ostringstream oss;
                oss << chan.getLimit();
                response += chan.getModes() + " Limit:" + oss.str();
            }
            else 
                response += chan.getModes();
            client.setResponse(response + "\n");
            client.sendResponse();
        }
        else if((client.getFullmsg()[2].size() > 1 && client.getFullmsg()[2][0] == '+') || client.getFullmsg()[2][0] == '-')
        {
            //MODE #channel +iklo 1234 50 
            std::vector<std::string> args; // y si sobran al final??
            for(size_t i = 3; i < client.getFullmsg().size(); i++)
                args.push_back(client.getFullmsg()[i]);
            
            std::map<char, std::string> cmap;
            std::string commands = client.getFullmsg()[2];
            char sign = commands[0];
            for(size_t i = 1; i < commands.size(); i++)
            {
				cmap[commands[i]] = "";
                if(args.size() > 0)
                {
                    if((commands[i] == 'o') || (sign == '+' && (commands[i] == 'k' || commands[i] == 'l')))
                    {
                        cmap[commands[i]] = args[0];
                        args.erase(args.begin());
                    }
                }
            }
            int i = 1;
            response += client.getHostname() + " MODE " + chan.getName() + " " + commands;
            //:dan!~h@localhost MODE #foobar +i
            // 
            // -ko pepe
			int flag = 1;
            while(commands[i])
            {
                if(commands.find_first_not_of("itklo", 1) != std::string::npos)
                    throw(ERR_UNKNOWNCOMMAND);
                if(!chan.isAMod(client.getNick()))
                        throw ERR_CHANOPRIVSNEEDED;
                if(commands[i] == 'i')
                    setModeInvite(sign, chan);
                else if(commands[i] == 't')
                    setModeTopic(sign,  chan);
                else if(commands[i] == 'k')
                {
					if(cmap['k'] == "" && sign == '+')
						throw(ERR_NEEDMOREPARAMS);
                    setModeKey(sign, chan, cmap['k']);
                }
                else if(commands[i] == 'l')
                {
					if(cmap['l'] == "" && sign == '+')
						throw(ERR_NEEDMOREPARAMS);
                    setModeLimit(sign, chan, cmap['l']);
                    response +=  " " + cmap['l'];
                }
                else if(commands[i] == 'o')
                {
					if(cmap['o'] == "")
						throw(ERR_NEEDMOREPARAMS);
                    flag = setModeModerator(sign, chan, cmap['o'], client);
                    response +=  " " + cmap['o'];
                }
                i++;
            }
			if(flag != -1)
            	chan.sendResponseChannel(response, client, 0);
			flag = 1;
            std::cout << GREEN << "Nº moderadores "<< chan.getModerators().size() << WHITE << std::endl;
        }
        else
            throw(ERR_UNKNOWNCOMMAND); //? las flags no llevan signo
        
    }
    catch(ERR num)
    {
        err(num, this->hostname, client, "");
    }
}

