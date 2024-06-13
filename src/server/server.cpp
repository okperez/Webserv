/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 16:56:17 by galambey          #+#    #+#             */
/*   Updated: 2024/06/13 17:34:59 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

struct pollfd *create_fds(int server_fd) {
	
	struct pollfd *fds;
	
	fds = new pollfd[MAX_CONNECTION];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN; // to set up the listen socket, ready to listen for new request from clients
	for (int i = 1; i < MAX_CONNECTION; i++) {
		fds[i].fd = -1;
		fds[i].events = 0;
		fds[i].revents = 0;
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
	if (socket_fd < 0)
	{
		close_fds(fds, MAX_CONNECTION);
		throw (ServerException("Failed to connect."));
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
	std::cerr << "Maximum of connection reached." << socket_fd << "." << std::endl;

}

/*
The normal procedure for parsing an HTTP message is to read the start-line
into a structure, read each header field line into a hash table by field name
until the empty line, and then use the parsed data to determine if a message
body is expected. If a message body has been indicated, then it is read as a
stream until an amount of octets equal to the message body length is read or the
connection is closed.
*/
void	do_request(struct pollfd *fds, int i, char *buffer) {

	t_request 	request;
	
	for (int j = 0; j < 1024; j++)
		std::cout << buffer[j];
	std::cout << std::endl;
	init_request_struct(request, buffer);
	handle_request(fds[i].fd, request);
}

/* Called if there something to be read and handled in one of the fds */
int	pollin_happen(struct pollfd *fds, int server_fd) {
	
	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		int pollin_happen = fds[i].revents & POLLIN;
		if (pollin_happen)
		{
			if (i == 0 )
				new_connection(fds, server_fd);
			else {
				char buffer[1024] = {0};
				int n_bytes = read(fds[i].fd, buffer, 1024); //secu si ==-1
				if (!n_bytes)
					close_connection(fds, i);
				else
					do_request(fds, i, buffer);
			}
			break;
		}
	}
	return (0);
}

int	launch_server(struct pollfd *fds, int server_fd) {
	
	
	while (1)
	{
		std::cout << "Waiting...\n";
		// Check si changement dans les fds (events/revents lies au fd(socket)) => si oui passe sinon attend
		int ret = poll(fds, MAX_CONNECTION, -1);
		if (ret < 0) { //verifier leak + leak de fd
			close_fds(fds, MAX_CONNECTION);
			throw(ServerException("Failed to poll."));
		}
		if (pollin_happen(fds, server_fd))
			return (1);
	}
	return (0);
}
