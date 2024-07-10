/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   close_server.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 15:50:27 by galambey          #+#    #+#             */
/*   Updated: 2024/07/10 16:08:00 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void	close_connection(struct pollfd *fds, int i) {

	std::cout << "Connection on socket " << fds[i].fd << " closed." << std::endl;
	close(fds[i].fd);
	fds[i].fd = -1;
	fds[i].revents = 0;
	fds[i].events = 0;
}

void	close_fds(struct pollfd *fds, int nb) {
	for (int i = 0; i < nb; i++) {
		if (fds[i].fd > -1)
			close_connection(fds, i);
	}
	delete[] fds;
}

void	save_fds(struct pollfd *fds, int max) {
	static	struct pollfd *fds_save;
	static	int nb;
	
	if (fds) {
		fds_save = fds;
		nb = max;
		return;
	}
	close_fds(fds_save, nb);
	throw (ServerException("exit"));
}

void	close_server(Server *server) {
	for (int i = 0; i < MAX_CONNECTION; i++) {
		server->fds[i].events = 0; // BLOQUE LES DEMANDE DE CONNECTION + ARRET DES LECTURES
	}
	server->handle_pending_requests();
	
	// for (std::vector<Listen>::iterator it = server->server_fd.begin(); it != server->server_fd.end(); it++) {
	// 	it
	// }
}

void	garbagge_server(Server *server) {
	static Server *server_save;

	if (server) {
		server_save = server;
		return ;
	}
	close_server(server_save);
	throw (ServerException("exit"));
}

void sighandler(int signal) {
	std::cout << std::endl << "Interruption of server due to signal " << signal << "." << std::endl;
	save_fds(NULL, 0);
}
