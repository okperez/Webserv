/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   close_server.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 15:50:27 by galambey          #+#    #+#             */
/*   Updated: 2024/06/13 17:34:43 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

void	close_fds(struct pollfd *fds, int nb) {
	for (int i = 0; i < nb; i++) {
		if (fds[i].fd > -1)
			close_connection(fds, i);
	}
	delete[] fds;
}

void	close_connection(struct pollfd *fds, int i) {

	std::cout << "Connection on socket " << fds[i].fd << " closed." << std::endl;
	close(fds[i].fd);
	fds[i].fd = -1;
	fds[i].revents = 0;
	fds[i].events = 0;
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

void sighandler(int signal) {
	std::cout << std::endl << "Interruption of server due to signal " << signal << "." << std::endl;
	save_fds(NULL, 0);
}
