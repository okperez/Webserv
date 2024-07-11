/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   close_server.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 15:50:27 by galambey          #+#    #+#             */
/*   Updated: 2024/07/11 10:35:26 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

static void	close_server(Server *server) {
	server->stop_listen(); // BLOQUE LES DEMANDE DE CONNECTION
	server->handle_pending_requests(); // Envoie erreur 500 a toutes les connections acceptees ou requests en cours
	throw (ServerException("exit"));
}

void	garbagge_server(Server *server, int rule) {
	static Server *server_save;

	if (server) {
		server_save = server;
		return ;
	}
	close_server(server_save);
	if (rule == PARENT)
		throw (ServerException("exit"));
	if (rule == CHILDREN)
		exit(1);
}

/* A TESTER : TOUS LES SIGNAUX */
void sighandler(int signal) {
	std::cout << std::endl << "Interruption of server due to signal " << signal << "." << std::endl;
	garbagge_server(NULL, PARENT);
}
