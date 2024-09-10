/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/09/10 11:39:19 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

int main(int argc, char **argv) 
{
	if (argc == 2) {
		try {
			Server server;
			try {
				garbagge_server(&server, 2);
				signal(SIGINT, sighandler);
				if (handle_conf_file(argv[1], server.conf) == -1)
					return (1);
				
				// E/A  A RAJOUTER DANS PARSING => VOIR AVEC ORLANDO : QU EST CE QU ON FAIT SI HOST N EXISTE PAS? ON LE DEFINIT PAR DEFAULT OU ON RENVOIE UNE ERREUR?
				
				server.open_listen_socket();
				server.create_fds();
				server.launch_server(server.getServer_fd_size() + MAX_CONNECTION);
				return (0);
				}
			catch (std::bad_alloc const & e) {
				std::cerr << e.what() << std::endl;
				server.error_bfr_launch();
				return (1);
			}
			catch (std::length_error const & e) {
				std::cerr << e.what() << std::endl;
				server.error_bfr_launch();
				return (1);
			}
			catch (std::out_of_range const & e) {
				std::cerr << e.what() << std::endl;
				server.error_bfr_launch();
				return (1);
			}
			catch (std::exception const & e) {
				std::string err = e.what();
				if (err == "exit")
					return (130);
				else
					std::cerr << e.what() << std::endl;
				return (1);
			}
		}
		catch (std::bad_alloc const & e) {
			std::cerr << e.what() << std::endl;
			return (1);
		}
		catch (std::length_error const & e) {
			std::cerr << e.what() << std::endl;
			return (1);
		}
		catch (std::out_of_range const & e) {
			std::cerr << e.what() << std::endl;
			return (1);
		}
		catch (std::exception const & e) {
			std::string err = e.what();
			if (err == "exit")
				return (130);
			else
				std::cerr << e.what() << std::endl;
			return (1);
		}
	}
    std::cerr << "Error: Missing configuration file: ./webserv [configuration file]\n";
	return (1);
}
