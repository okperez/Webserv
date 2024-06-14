/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_gate.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 09:15:19 by galambey          #+#    #+#             */
/*   Updated: 2024/06/14 12:20:28 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

void	open_listen_socket(t_conf &conf, std::vector<int> &server_fd) {
	
	(void) conf;
	/* QUI SERA REMPLACE PAR LES ELEMENTS DE CONF :*/	
	std::vector<int> ipv4_port;
	ipv4_port.reserve(4);
	for (int i = 8080; i < 8084; i++)
		ipv4_port.push_back(i);
	// std::vector<int> ipv6_port; => A gerer peut etre?
	
	/* QUI RESTE :*/	
	server_fd.reserve(ipv4_port.size());
	std::vector<int>::iterator it_s = server_fd.begin();
	for(std::vector<int>::iterator it = ipv4_port.begin(); it != ipv4_port.end(); it++) {
		
		int new_socket = socket(AF_INET, SOCK_STREAM, 0);    //socket set up for listening is used only for accepting connections, not for exchanging data
		if (new_socket < 0)
			throw (ServerException("Failed to create server socket"));
	
		struct sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;          // address family
		server_addr.sin_addr.s_addr = INADDR_ANY;   //The address for this socket. This is just your machine’s IP address
		server_addr.sin_port = htons(*it);         //The port number (the transport address)
		if (bind(new_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) { // assigning a transport address to the socket
			for(it_s = server_fd.begin(); it_s != server_fd.end(); it_s++)
				close (*it_s);
			std::cerr << "Failed to bind to port " << *it << std::endl;
			throw(ServerException(""));
		}
		int connection_backlog = 10; // VOIR AVEC ORLANDO POURQUOI 10? A MODIFIER?
		if (listen(new_socket, connection_backlog) != 0) {   //The listen system call tells a socket that it should be capable of accepting incoming connections:
		      											//backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
			for(it_s = server_fd.begin(); it_s != server_fd.end(); it_s++)
				close (*it_s);
			std::cerr << "Failed to listen to port " << *it << std::endl;
			throw(ServerException(""));
		}
		/* To set up non blocking listen socket */
		int flags = fcntl(new_socket, F_GETFL, 0); // A VERIFIER POUR FNCTL SUJET + ORLANDO
		fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);
		server_fd.push_back(new_socket);
	}	
}
