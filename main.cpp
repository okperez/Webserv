/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/06/11 15:26:16 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int main(int argc, char **argv) 
{
    if (argc == 2)
    {
    	if (parse_conf_file(argv[1]) == -1)
    		return (1);
		
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
    	struct sockaddr_in client_addr;
    	int client_addr_len = sizeof(client_addr);
    	std::cout << "Waiting for a client to connect...\n";
    	//he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
    	//By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
    	int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    	if (socket_fd < 0)
    	{
    	  std::cerr << "Failed to connect\n";
    	  return (1);
    	}
    	std::cout << "Client connected\n";
//  	 --------------------------------------------------------------------
//  	 This part has successfully created TCP socket on the server computer
//  	 --------------------------------------------------------------------
    	char buffer[1024] = {0};
    	read(socket_fd, buffer, 1024);
    	/*The normal procedure for parsing an HTTP message is to read the start-line
    	into a structure, read each header field line into a hash table by field name
    	until the empty line, and then use the parsed data to determine if a message
    	body is expected. If a message body has been indicated, then it is read as a
    	stream until an amount of octets equal to the message body length is read or the
    	connection is closed.*/
    	init_request_struct(request, buffer);
    	// std::cout << "buffer = " << buffer;
    	handle_request(socket_fd, request);
    	close(server_fd);
    	close(socket_fd);
  		return (0);
	}
    std::cerr << "Error: Missing configuration file: ./webserv [configuration file]\n";
	return (1);
}