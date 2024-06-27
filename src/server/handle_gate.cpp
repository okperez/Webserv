/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_gate.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 09:15:19 by galambey          #+#    #+#             */
/*   Updated: 2024/06/27 11:00:16 by galambey         ###   ########.fr       */
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

// /* Assigning a transport address to the socket */
// void	bind_socket(t_listen *new_socket, struct sockaddr &server_addr, std::vector<t_listen> &server_fd, int port) {
	
// 	if (bind((*new_socket).fd, (struct sockaddr *) &server_addr, sizeof(server_addr)))
// 	{
// 		for(std::vector<t_listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
// 			close (it->fd);
// 		std::cerr << "Failed to bind to port " << port << std::endl;
// 		throw(ServerException(""));
// 	}
// }

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
	int opt = 1;
	/* QUI SERA REMPLACE PAR LES ELEMENTS DE CONF :*/	
	// std::vector<t_conftest> conf_test(2);
	// conf_test[0].ipv4_port.reserve(4);
	// conf_test[1].ipv4_port.reserve(4);
	// conf_test[0].ipv4_port.push_back("8180");
	// conf_test[0].ipv4_port.push_back("8181");
	// conf_test[0].ipv4_port.push_back("8182");
	// conf_test[0].ipv4_port.push_back("8183");
	// conf_test[1].ipv4_port.push_back("8184");
	// conf_test[1].ipv4_port.push_back("8185");
	// conf_test[1].ipv4_port.push_back("8186");
	// std::vector<int> ipv6_port; => A gerer peut etre?
	
	// A REMPLACER : CONF_TEST PAR CONF
	/* QUI RESTE :*/
	// for(std::vector<t_conftest>::iterator it = conf_test.begin(); it != conf_test.end(); it++) {
	for(std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
		for(std::vector<std::string>::iterator jt = it->ipv4_port.begin(); jt != it->ipv4_port.end(); jt++) {
			int port;
			std::istringstream iss(*jt);
			iss >> port;
			if (check_port_binding(server_fd, port))
				continue;
			t_listen new_socket;
			new_socket.fd = socket(AF_INET, SOCK_STREAM, 0);   //socket set up for listening is used only for accepting connections, not for exchanging data
			if (new_socket.fd < 0)
				throw (ServerException("Failed to create server socket"));
			/*
			Le flag SO_REUSEADDR permet d'assigner la socket a un port meme si ce dernier est en TIME_WAIT
			ce qui peut arriver quand le serveur est quitte via un CTRL+C.
			Comme ca permet d'autoriser la réutilisation des adresses locales, il faut faire attention
			a son utillisation car cela pourrait engendrer des fqilles de securite. 
			*/
			if (setsockopt(new_socket.fd, SOL_SOCKET, SO_REUSEADDR, &opt, 4))
				throw (ServerException("Failed to create server socket"));
			struct addrinfo hints, *res;
			struct sockaddr_in *server;
			
			hints.ai_family = AF_INET;          			// address family
			hints.ai_socktype = SOCK_STREAM;          			
			getaddrinfo("127.0.0.2", NULL, &hints, &res);
			server = (struct sockaddr_in *)res->ai_addr;
			server->sin_port = htons(port);         			//The port number (the transport address)
			bind_socket(&new_socket, *server, server_fd, port);
			listen_socket(&new_socket, server_fd, port);
			
			// struct sockaddr_in server_addr;
			// server_addr.sin_family = AF_INET;          			// address family
			// server_addr.sin_addr.s_addr = INADDR_ANY;   //inetadress du host 		//The address for this socket. This is just your machine’s IP address
			// server_addr.sin_port = htons(port);         			//The port number (the transport address)
			// bind_socket(&new_socket, server_addr, server_fd, port);
			// listen_socket(&new_socket, server_fd, port);
			new_socket.port = port;
			server_fd.push_back(new_socket);
		}
	}
}
