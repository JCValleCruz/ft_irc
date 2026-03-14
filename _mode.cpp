/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _mode.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvalle-d <jvalle-d@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 12:50:49 by jormoral          #+#    #+#             */
/*   Updated: 2025/10/08 19:36:33 by jvalle-d         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"

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
            std::vector<std::string> mode_tokens;
            std::vector<std::string> args;
            for(size_t i = 2; i < client.getFullmsg().size(); i++)
            {
                if(client.getFullmsg()[i][0] == '+' || client.getFullmsg()[i][0] == '-')
                    mode_tokens.push_back(client.getFullmsg()[i]);
                else
                    args.push_back(client.getFullmsg()[i]);
            }
            response += client.getHostname() + " MODE " + chan.getName();
            int flag = 1;
            for(size_t mt = 0; mt < mode_tokens.size(); mt++)
            {
                std::map<char, std::string> cmap;
                std::string commands = mode_tokens[mt];
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
                response += " " + commands;
                while(commands[i])
                {
                    if(commands.find_first_not_of("itklo", 1) != std::string::npos)
                        throw(ERR_UNKNOWNCOMMAND);
                    if(!chan.isAMod(client.getNick()))
                        throw ERR_CHANOPRIVSNEEDED;
                    if(commands[i] == 'i')
                        setModeInvite(sign, chan);
                    else if(commands[i] == 't')
                        setModeTopic(sign, chan);
                    else if(commands[i] == 'k')
                    {
                        if(cmap['k'] == "" && sign == '+')
                            throw(ERR_NEEDMOREPARAMS);
                        setModeKey(sign, chan, cmap['k']);
                        if(sign == '+')
                            response += " " + cmap['k'];
                    }
                    else if(commands[i] == 'l')
                    {
                        if(cmap['l'] == "" && sign == '+')
                            throw(ERR_NEEDMOREPARAMS);
                        setModeLimit(sign, chan, cmap['l']);
                        response += " " + cmap['l'];
                    }
                    else if(commands[i] == 'o')
                    {
                        if(cmap['o'] == "")
                            throw(ERR_NEEDMOREPARAMS);
                        flag = setModeModerator(sign, chan, cmap['o'], client);
                        response += " " + cmap['o'];
                    }
                    i++;
                }
            }
            if(flag != -1)
                chan.sendResponseChannel(response, client, 0);
            flag = 1;
            std::cout << GREEN << "Nº moderadores "<< chan.getModerators().size() << WHITE << std::endl;
        }
        else
            throw(ERR_UNKNOWNCOMMAND);
        
    }
    catch(ERR num)
    {
        err(num, this->hostname, client, "");
    }
}

