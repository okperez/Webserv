/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_gate.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 09:15:19 by galambey          #+#    #+#             */
/*   Updated: 2024/06/14 17:49:21 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

/*
Si une socket ecoute deja sur le port, retourne 1 pour continuer et ne pas creer la socket
*/
bool	check_port_binding(std::vector<t_listen> &server_fd, int port) {
	for(std::vector<t_listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
		if(it->port == port)
			return (true);
	}
	return (false);
}


/* Assigning a transport address to the socket */
void	bind_socket(t_listen *new_socket, struct sockaddr_in &server_addr, std::vector<t_listen> &server_fd, int port) {
	
	if (bind((*new_socket).fd, (struct sockaddr *) &server_addr, sizeof(server_addr)))
	{
		for(std::vector<t_listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
			close (it->fd);
		std::cerr << "Failed to bind to port " << port << std::endl;
		throw(ServerException(""));
	}
}

/*
The listen system call tells a socket that it should be capable of accepting incoming connections:
backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
*/
void	listen_socket(t_listen *new_socket, std::vector<t_listen> &server_fd, int port) {
	
	int connection_backlog = 10; // VOIR AVEC ORLANDO POURQUOI 10? A MODIFIER?
	if (listen((*new_socket).fd, connection_backlog) != 0) {
		for(std::vector<t_listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
			close (it->fd);
		std::cerr << "Failed to listen to port " << port << std::endl;
		throw(ServerException(""));
	}
	/* To set up non blocking listen socket */
	int flags = fcntl((*new_socket).fd, F_GETFL, 0); // A VERIFIER POUR FNCTL SUJET + ORLANDO
	fcntl((*new_socket).fd, F_SETFL, flags | O_NONBLOCK);
}

void	open_listen_socket(std::vector<t_conf> &conf, std::vector<t_listen> &server_fd) {
	
	(void) conf;
	/* QUI SERA REMPLACE PAR LES ELEMENTS DE CONF :*/	
	std::vector<t_conftest> conf_test(2);
	std::vector<int> ipv4_port;
	conf_test[0].ipv4_port.reserve(4);
	conf_test[1].ipv4_port.reserve(4);
	for (int i = 8080; i < 8084; i++)
		conf_test[0].ipv4_port.push_back(i);
	for (int i = 8082; i < 8086; i++)
		conf_test[1].ipv4_port.push_back(i);
	// std::vector<int> ipv6_port; => A gerer peut etre?
	
	// A REMPLACER : CONF_TEST PAR CONF
	/* QUI RESTE :*/
	for(std::vector<t_conftest>::iterator it = conf_test.begin(); it != conf_test.end(); it++) {
		for(std::vector<int>::iterator jt = it->ipv4_port.begin(); jt != it->ipv4_port.end(); jt++) {
			if (check_port_binding(server_fd, *jt))
				continue;
			t_listen new_socket;
			new_socket.fd = socket(AF_INET, SOCK_STREAM, 0);   //socket set up for listening is used only for accepting connections, not for exchanging data
			if (new_socket.fd < 0)
				throw (ServerException("Failed to create server socket"));
		
			struct sockaddr_in server_addr;
			server_addr.sin_family = AF_INET;          			// address family
			server_addr.sin_addr.s_addr = INADDR_ANY;   		//The address for this socket. This is just your machine’s IP address
			server_addr.sin_port = htons(*jt);         			//The port number (the transport address)
			bind_socket(&new_socket, server_addr, server_fd, *jt);
			listen_socket(&new_socket, server_fd, *jt);
			new_socket.port = *jt;
			server_fd.push_back(new_socket);
		}
	}
}
