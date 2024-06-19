/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/06/19 10:59:00 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

int main(int argc, char **argv) 
{
	try {
		if (argc == 2)
		{
			std::vector<t_conf>	conf;
			signal(SIGINT, sighandler);
			if (handle_conf_file(argv[1], conf) == -1)
				return (1);

			/* ********** A EFFACER ************ */

			// print_location(conf);
			// std::cout << "conf.root_dir " << conf[0].root_dir << std::endl;
			
			// t_conf		test;

			// test.server_name = "test";
			// conf.push_back(test);
			
			/* ********************************* */
			
			// return (1);
			
			std::vector<t_listen> server_fd;
			
			/* Il va falloir qu on sache pour chaque fd quel serveur */
			open_listen_socket(conf, server_fd);
			struct pollfd *fds = create_fds(server_fd); //tableau de struct
			save_fds(fds, server_fd.size() + MAX_CONNECTION);
			launch_server(fds, server_fd, server_fd.size() + MAX_CONNECTION, conf);
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