/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 16:56:17 by galambey          #+#    #+#             */
/*   Updated: 2024/06/18 18:40:38 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

struct pollfd *create_fds(std::vector<t_listen> &server_fd) {
	
	struct pollfd *fds;
	int i = 0;
	
	fds = new pollfd[MAX_CONNECTION + server_fd.size()];
	// ou max de co par serveur ? 
	// fds = new pollfd[MAX_CONNECTION * server_fd.size() + server_fd.size()];
	for (std::vector<t_listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
		fds[i].fd = it->fd;
		fds[i].events = POLLIN; // to set up the listen socket, ready to listen for new request from clients
		i++;
	}
	while( i < MAX_CONNECTION + server_fd.size()) {
		fds[i].fd = -1;
		fds[i].events = 0;
		fds[i].revents = 0;
		i++;
	}
	return (fds);
}

void	new_connection(struct pollfd *fds, int server_fd) {
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	
	std::cout << "Client asked for connection." << std::endl;
	// A VERIF SI TOUJOURS LE CAS OU NON : 
		// he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
		// By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
	int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
	if (socket_fd < 0) {
		std::cout << "Client failed to connect." << std::endl;
		return ;
	}
	for (int i = 1; i < MAX_CONNECTION; i++) {
		if (fds[i].fd == -1) {
			fds[i].fd = socket_fd;
			fds[i].events = POLLIN;
			std::cout << "Client connected on socket " << socket_fd << "." << std::endl;
			return ;
		}
	}
	close(socket_fd);
	std::cerr << "Maximum of connection reached." << std::endl;
	std::cerr << "Connection on socket " << socket_fd << " closed." << std::endl;

}

/* Called if there something to be read and handled in one of the fds */
void	pollin_happen(struct pollfd *fds, std::vector<t_listen> &server_fd, std::vector<t_conf> & conf, std::map<std::string, std::string> map_error) {
	
	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		int pollin_happen = fds[i].revents & POLLIN;
		if (pollin_happen)
		{
			/* pollin_happen sur socket listening for clients */
			for (std::vector<t_listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
				if (it->fd == fds[i].fd) {
					new_connection(fds, it->fd);
					return ;
				}
			}
			/* pollin_happen sur socket listening for request ready to be handled */
			char buffer[1024] = {0};
			int n_bytes = read(fds[i].fd, buffer, 1024); //secu si ==-1
			if (!n_bytes)
				close_connection(fds, i);
			else
				do_request(fds, i, buffer, conf, map_error);
			return ;
		}
	}
}

void	launch_server(struct pollfd *fds, std::vector<t_listen> &server_fd, int max_socket, std::vector<t_conf> & conf) {
	
	/* Est ce qu on ne ferait pas une struct avec map_error et vector_server ?*/
	std::map<std::string, std::string> map_error;
	create_map_error(map_error);
	while (1)
	{
		std::cout << "Waiting...\n";
		// Check si changement dans les fds (events/revents lies au fd(socket)) => si oui passe sinon attend
		int ret = poll(fds, max_socket, -1);
		if (ret < 0) { //verifier leak + leak de fd
			close_fds(fds, max_socket);
			throw(ServerException("Failed to poll."));
		}
		pollin_happen(fds, server_fd, conf, map_error);
	}
}
