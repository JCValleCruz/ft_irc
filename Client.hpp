/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aehrl <aehrl@student.42malaga.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:57:24 by jvalle-d          #+#    #+#             */
/*   Updated: 2026/03/22 21:04:12 by aehrl            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CLIENT_HPP
#define CLIENT_HPP 

#include "Server.hpp"
#include "Channel.hpp"
class Channel;

class Client{
private:
	int socket;
	sockaddr_in address;
	socklen_t a_len;
	std::string nick;
	std::string user;
	std::string realname;
	bool verified;
	bool username_verified;
	
	
	std::string message;
	std::string argument;
	
	std::string response;
	int retries;
	
	struct hostent* host;
	std::string hostname;
	std::vector<std::string> fullmsg;
	
	public:
		bool changeNick;
		Client(int socket, sockaddr_in address);
		Client();
		~Client();
		int getSocket();
		void setSocket(int);
	
		void setMessage(std::string message);
		std::string getMessage();

		void setArgument(std::string argument);
		std::string getArgument();

		void setResponse(std::string responset);
		std::string getResponse();
		void sendResponse();

		void setNick(std::string nick);
		std::string getNick();

		bool getVerify();
		void setVerify(bool veri);


		void decreaseRetries();
		int getRetries();

		bool getUserVerified();
		std::string getUsername();
		void		setUsername(std::string user);

		std::string getHostname();
		void setHostname();
		void setHostnameManual(std::string hostname);
		
		std::vector<std::string> getFullmsg();
		void setFullmsg(std::vector<std::string>);
		void printMessage();
};

#endif