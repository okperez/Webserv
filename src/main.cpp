/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/06/12 14:53:53 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

#define MAX_CONNECTION	20
int main(int argc, char **argv) 
{
    if (argc == 2)
    {
    	if (parse_conf_file(argv[1]) == -1)
			std::cout  << "a" << std::endl;				
    		// return (1);
		
    	t_request request;
    	t_conf    conf;
    	int server_fd = socket(AF_INET, SOCK_STREAM, 0);    //socket set up for listening is used only for accepting connections, not for exchanging data
    	if (server_fd < 0) {
    	 std::cerr << "Failed to create server socket\n";
    	 return 1;
    	}
    	struct sockaddr_in server_addr;
    	server_addr.sin_family = AF_INET;          // address family
    	server_addr.sin_addr.s_addr = INADDR_ANY;   //The address for this socket. This is just your machine’s IP address
    	server_addr.sin_port = htons(4221);         //The port number (the transport address)
    	if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) { // assigning a transport address to the socket
    	  std::cerr << "Failed to bind to port 4221\n";
    	  return 1;
    	}
    	int connection_backlog = 5;
    	if (listen(server_fd, connection_backlog) != 0) {   //The listen system call tells a socket that it should be capable of accepting incoming connections:
    	  std::cerr << "listen failed\n";                   //backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
    	  return 1;
    	}
/********************/
		struct pollfd *fds; //tableau de struct
		try {
			fds = new pollfd[MAX_CONNECTION];
			for (int i = 1; i < MAX_CONNECTION; i++)
				fds[i].fd = -1;
		}
		catch (std::bad_alloc const & e) {
			std::cout << "Failed to allocate memmory;" << std::endl;
			return (1);
		}
		/* To set up non blocking listen socket */
		int flags = fcntl(server_fd, F_GETFL, 0);
		fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
		
		fds[0].fd = server_fd;
		fds[0].events = POLLIN; // si on fait ca : 20 connections se font au lieu d une seule
		// int ret = poll(fds, MAX_CONNECTION, -1); 	// tant que un des event/revent (request/response) lie a 1 fd ne sont pas 
										// mis a jour, poll va attendre (ce qui permet notamment
										// de ne pas faire quelque chose pour rien / optimisation
										// des cpu). Une fois qu on sort du poll ca veut dire qu'il y a un changement
										// dans le "status" des event/revent et donc quelque chose a faire
		// if (ret < 0) { //verifier leak + leak de fd
    	//   std::cerr << "Failed to poll\n";
		// 	return (1);
		// }
		while (1)
		{
			struct sockaddr_in client_addr;
			int client_addr_len = sizeof(client_addr);
			std::cout << "Waiting for a client to connect...\n";

			int ret = poll(fds, MAX_CONNECTION + 1, -1); 	// tant que un des event/revent (request/response) lie a 1 fd ne sont pas 
											// mis a jour, poll va attendre (ce qui permet notamment
											// de ne pas faire quelque chose pour rien / optimisation
											// des cpu). Une fois qu on sort du poll ca veut dire qu'il y a un changement
											// dans le "status" des event/revent et donc quelque chose a faire
			std::cout << "poll passed" << std::endl;
			if (ret < 0) { //verifier leak + leak de fd
			std::cerr << "Failed to poll\n";
				return (1);
			}
			
			//he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
			//By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
			// int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
			// if (socket_fd < 0)
			// {
			// std::cerr << "Failed to connect\n";
			// return (1);
			// }
			for (int j = 0; j < MAX_CONNECTION; j++)
			{
				int i;
				int pollin_happen = fds[j].revents & POLLIN;
				std::cout << pollin_happen << std::endl;
				if (pollin_happen)
				{
					if (j == 0)
					{
						std::cout << "file ready to read" << std::endl;
						int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
						if (socket_fd < 0)
						{
							std::cerr << "Failed to connect\n";
							return (1);
						}
						for (i = 1; i < MAX_CONNECTION; i++) {
							if (fds[i].fd == -1) {
								fds[i].fd = socket_fd;
								fds[i].events = POLLIN;
								std::cout << "Client connected on fds[" << i << "] soit fd = " << socket_fd << std::endl;
								// char buffer[1024] = {0};
								// read(fds[j].fd, buffer, 1024);
								// for (int k = 0; k < 1024; k++)
								// 	std::cout << buffer[k];
								// std::cout << std::endl;
								// init_request_struct(request, buffer);
								fds[0].events = POLLIN;
								break;
								
							}
						}
			//  	 --------------------------------------------------------------------
			//  	 This part has successfully created TCP socket on the server computer
			//  	 --------------------------------------------------------------------
						// char buffer[1024] = {0};
						// read(fds[j].fd, buffer, 1024);
						// init_request_struct(request, buffer);
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
						// read(fds[j].fd, buffer, 1024);
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
							// std::cout << "buffer = " << buffer;
							handle_request(fds[j].fd, request);
							// fds[j].revents = 0;
							// fds[j].events = 0;
						}
						// fds[j].revents = 0;
						// fds[j].events = 0;
					}
					// handle_request(socket_fd, request);
					break;
				}
			}
		}
    	close(server_fd);
    	// close(socket_fd);
		// for () => A IMPLEMENTER : boucle pour fermer les sockets */
  		return (0);
	}
    std::cerr << "Error: Missing configuration file: ./webserv [configuration file]\n";
	return (1);
}