/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/06/14 12:24:51 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

int main(int argc, char **argv) 
{
	try {
		if (argc == 2)
		{
			signal(SIGINT, sighandler);
			if (parse_conf_file(argv[1]) == -1)
				;
				// return (1);
			
			t_conf    conf;
			std::vector<int> server_fd;

			open_listen_socket(conf, server_fd);
			
			struct pollfd *fds = create_fds(server_fd); //tableau de struct
			save_fds(fds, MAX_CONNECTION);
			launch_server(fds, server_fd);
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