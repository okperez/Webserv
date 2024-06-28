/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:43:55 by galambey          #+#    #+#             */
/*   Updated: 2024/06/28 18:17:58 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Server::Server() {}

Server::Server(const Server & orig) { (void) orig; }

Server::~Server() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */

/* ************************** Assignment Operator  ************************* */

Server &Server::operator=(Server & rhs) {
	(void) rhs;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */


/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

/* ************************************************************************* */
/* ***************************** BFR LAUNCHING ***************************** */
/* ************************************************************************* */

/* Assigning a transport address to the socket */
void	Server::bind_socket(int new_socket, struct sockaddr_in &server_addr, int port) {
	
	if (bind(new_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)))
	{
		for(std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
			it->close_fd();
			// close (it->_fd);
		std::cerr << "Failed to bind to port " << port << std::endl;
		throw(ServerException(""));
	}
}

/*
The listen system call tells a socket that it should be capable of accepting incoming connections:
backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
*/
void	Server::listen_socket(int new_socket, int port) {
	
	int connection_backlog = 10; // VOIR AVEC ORLANDO POURQUOI 10? A MODIFIER?
	if (listen(new_socket, connection_backlog) != 0) {
		for(std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
			it->close_fd();
			// close (it->fd);
		std::cerr << "Failed to listen to port " << port << std::endl;
		throw(ServerException(""));
	}
	/* To set up non blocking listen socket */
	int flags = fcntl(new_socket, F_GETFL, 0); // A VERIFIER POUR FNCTL SUJET + ORLANDO
	fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);
}
	
void	Server::open_listen_socket() {
	
	int opt = 1;
	int i = 0;
	/* QUI SERA REMPLACE PAR LES ELEMENTS DE CONF :*/	
	
	/* QUI RESTE :*/
	// for(std::vector<t_conftest>::iterator it = conf_test.begin(); it != conf_test.end(); it++) {
	for(std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
		for(std::vector<std::string>::iterator jt = it->ipv4_port.begin(); jt != it->ipv4_port.end(); jt++) {
			int port;
			std::istringstream iss(*jt);
			iss >> port;
			// if (check_port_binding(server_fd, port, it->host)) {
			// 	std::cout << "CONTINUE" << std::endl;
			// 	continue;
			// }
			int new_socket;
			new_socket = socket(AF_INET, SOCK_STREAM, 0);   //socket set up for listening is used only for accepting connections, not for exchanging data
			if (new_socket < 0)
				throw (ServerException("Failed to create server socket"));
			/*
			Le flag SO_REUSEADDR permet d'assigner la socket a un port meme si ce dernier est en TIME_WAIT
			ce qui peut arriver quand le serveur est quitte via un CTRL+C.
			Comme ca permet d'autoriser la réutilisation des adresses locales, il faut faire attention
			a son utillisation car cela pourrait engendrer des fqilles de securite. 
			*/
			if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &opt, 4))
				throw (ServerException("Failed to create server socket"));
			struct addrinfo hints, *res;
			struct sockaddr_in *server;
			
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;          			// address family
			hints.ai_socktype = SOCK_STREAM;          			
			int result = getaddrinfo(it->host.data(), NULL, &hints, &res); // => Permet de lier l'adresse IP de l hote au port et a la socket via la structure sockaddr_in
			if (result != 0)
				throw(ServerException("Non valid host"));
			server = (struct sockaddr_in *)res->ai_addr;
			std::cout << "addresse de server->sin_addr : " << server->sin_addr.s_addr << std::endl;
			server->sin_port = htons(port);         			//The port number (the transport address)
			this->bind_socket(new_socket, *server, port);
			this->listen_socket(new_socket, port);
			Listen nw(new_socket, *jt, server->sin_addr.s_addr, it->host, i);
			freeaddrinfo(res);
			server_fd.push_back(nw);
		}
		i++;
	}
}

void	Server::create_fds() {
	
	int i = 0;
	
	fds = new pollfd[MAX_CONNECTION + server_fd.size()];
	// ou max de co par serveur ? 
	// fds = new pollfd[MAX_CONNECTION * server_fd.size() + server_fd.size()];
	for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
		fds[i].fd = it->getFd();
		fds[i].events = POLLIN; // to set up the listen socket, ready to listen for new request from clients
		i++;
	}
	while( i < MAX_CONNECTION + server_fd.size()) {
		fds[i].fd = -1;
		fds[i].events = 0;
		fds[i].revents = 0;
		i++;
	}
}
