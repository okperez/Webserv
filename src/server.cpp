/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 16:56:17 by galambey          #+#    #+#             */
/*   Updated: 2024/06/12 17:40:48 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

struct pollfd *create_fds(int server_fd) {
	
	struct pollfd *fds;
	
	fds = new pollfd[MAX_CONNECTION];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN; // to set up the listen socket, ready to listen for new request from clients
	for (int i = 1; i < MAX_CONNECTION; i++)
		fds[i].fd = -1;
	return (fds);
}

int	new_connection(struct pollfd *fds, int server_fd) {
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	
	std::cout << "Client asked for connection" << std::endl;
	// A VERIF SI TOUJOURS LE CAS OU NON : 
		// he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
		// By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
	int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
	if (socket_fd < 0)
	{
		std::cerr << "Failed to connect\n";
		return (1);
	}
	for (int i = 1; i < MAX_CONNECTION; i++) {
		if (fds[i].fd == -1) {
			fds[i].fd = socket_fd;
			fds[i].events = POLLIN;
			std::cout << "Client connected on socket " << socket_fd << std::endl;
			break;
		}
	}
	return (0);
}

/* Called if there something to be read and handled in one of the fds */
int	pollin_happen(struct pollfd *fds, int server_fd) {
	
	t_request 	request;
	
	for (int j = 0; j < MAX_CONNECTION; j++)
	{
		int pollin_happen = fds[j].revents & POLLIN;
		std::cout << pollin_happen << std::endl;
		if (pollin_happen)
		{
			if (j == 0 )
			{
				if (new_connection(fds, server_fd) == 1)
					return (1);
	//  	 --------------------------------------------------------------------
	//  	 This part has successfully created TCP socket on the server computer
	//  	 --------------------------------------------------------------------
			}
			else {
				char buffer[1024] = {0};
				int n_bytes = read(fds[j].fd, buffer, 1024); //secu si ==-1
				if (n_bytes == 0) {
					std::cout << "nothing in buff" << std::endl;
					close(fds[j].fd);
					fds[j].fd = -1;
					fds[j].revents = 0;
					fds[j].events = 0;
				}
				else
				{
					for (int k = 0; k < 1024; k++)
						std::cout << buffer[k];
					std::cout << std::endl;
					/*The normal procedure for parsing an HTTP message is to read the start-line
					into a structure, read each header field line into a hash table by field name
					until the empty line, and then use the parsed data to determine if a message
					body is expected. If a message body has been indicated, then it is read as a
					stream until an amount of octets equal to the message body length is read or the
					connection is closed.*/
					init_request_struct(request, buffer);
					handle_request(fds[j].fd, request);
				}
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
		int ret = poll(fds, MAX_CONNECTION + 1, -1);
		if (ret < 0) { //verifier leak + leak de fd
			std::cerr << "Failed to poll\n";
			return (1);
		}
		if (pollin_happen(fds, server_fd))
			return (1);
	}
	return (0);
}