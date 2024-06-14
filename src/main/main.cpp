/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/06/14 18:11:13 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

// void	save_fds(struct pollfd *fds, int max) {
// 	static	struct pollfd *fds_save;
// 	static	int nb;
	
// 	if (fds) {
// 		fds_save = fds;
// 		nb = max;
// 		return;
// 	}
// 	for (int i = 0; i < nb; i++) {
// 		if (fds_save[i].fd > -1)
// 			close_connection(fds_save, i);
// 	}
// 	delete[] fds_save;
// 	throw (ServerException("exit"));
// }

// void sighandler(int signal) {
// 	std::cout << std::endl << "Interruption of server due to signal " << signal << std::endl;
// 	save_fds(NULL, 0);
// }

int main(int argc, char **argv) 
{
	try {
		if (argc == 2)
		{
			std::vector<t_conf>	conf;
			signal(SIGINT, sighandler);
			if (parse_conf_file(argv[1], conf) == -1)
				;
				// return (1);
			
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
			close (server_fd);
			return 1;
			}
			int connection_backlog = 5;
			if (listen(server_fd, connection_backlog) != 0) {   //The listen system call tells a socket that it should be capable of accepting incoming connections:
			std::cerr << "listen failed\n";                   //backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
			return 1;
			}
			// /* To set up non blocking listen socket */
			int flags = fcntl(server_fd, F_GETFL, 0);
			fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
	/********************/
			struct pollfd *fds = create_fds(server_fd); //tableau de struct
			save_fds(fds, MAX_CONNECTION);
			int ret = launch_server(fds, server_fd);
			close(server_fd);
			if (ret == 1)
				return (1);
			// close(socket_fd);
			// for () => A IMPLEMENTER : boucle pour fermer les sockets */
			return (0);
		}
	}
	catch (std::exception const & e) {
		std::string err = e.what();
		if (err == "exit")
			return (130);
		else
			std::cerr << e.what() << std::endl;
		return (1);
	}
    std::cerr << "Error: Missing configuration file: ./webserv [configuration file]\n";
	return (1);
}