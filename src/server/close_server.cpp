/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   close_server.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 15:50:27 by galambey          #+#    #+#             */
/*   Updated: 2024/09/10 10:10:04 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

static void	close_server(Server *server) {
	server->setCtrlC();
	throw (ServerException("exit"));
}

void	closeChild(Server *server)
{
	server->setTimeSigint();
}

void	garbagge_server(Server *server, int rule) {
	static Server *server_save;
	(void) rule;

	if (server) {
		server_save = server;
		return ;
	}
	if (rule == PARENT) {
		close_server(server_save);
	}
	if (rule == CHILDREN)
	{
		closeChild(server_save);		
	}
}

/* A TESTER : TOUS LES SIGNAUX */
void sighandler(int signal) {
	std::cout << std::endl << "Interruption of server due to signal " << signal << "." << std::endl;
	garbagge_server(NULL, PARENT);
}

void sighandlerbis(int signal) {
	std::cout << std::endl << "Interruption of server due to signal " << signal << "." << std::endl;
	garbagge_server(NULL, CHILDREN);
}
