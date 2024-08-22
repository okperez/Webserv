/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/08/22 10:02:02 by galambey         ###   ########.fr       */
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
				// std::vector<t_conf>	conf;
				signal(SIGINT, sighandler);
				if (handle_conf_file(argv[1], server.conf) == -1)
					return (1);
				
				// E/A  A RAJOUTER DANS PARSING HOST : si fichier conf 127.000.000.001 => doit devenir 127.0.0.1 : PS fonction adapt_host ci dessus
				// E/A  A RAJOUTER DANS PARSING => VOIR AVEC ORLANDO : QU EST CE QU ON FAIT SI HOST N EXISTE PAS? ON LE DEFINIT PAR DEFAULT OU ON RENVOIE UNE ERREUR?
				// A RAJOUTER DANS PARSING : directive autoindex et directive return dans bloc server 
				
				for (std::vector<t_conf>::iterator it =server.conf.begin(); it != server.conf.end(); it++) {
					if (it->host.empty())
						it->host = "0.0.0.0";
					try {
						it->limit_body_size = ft_stoi(it->max_body_size) ;
					}
					catch (std::exception const & e) {
						throw (ConfFileException("Error: body size isn't a number"));
					}
				}

				// SI PAS DE SERVER NAME OU SI SERVER NAME VIDE ==== > LEAKS SI RETURN(-1) ICI
				std::cout << "******************************** END PARSING ********************************* " << std::endl;
				/* ********** A EFFACER ************ */
				std::cout << "body_size = " << server.conf[0].max_body_size << std::endl;
				server.conf[0].autoindex = "on";
				// server.conf[1].autoindex = "off";
				
				/* ********************************* */
				
				server.open_listen_socket();  	// NO LEAKS MEMMORY + FD OK
				server.create_fds();   			// NO LEAKS MEMMORY + FD OK
				server.launch_server(server.server_fd.size() + MAX_CONNECTION); // OK UNE FOIS LANCER ON RENTRE DANS CATCH QU AVEC CTRL+C OU FAIL DE POLL
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

// int main(int argc, char **argv) 
// {
// 	try {
// 		if (argc == 2)
// 		{
// 			Server server;
// 			garbagge_server(&server, 2);
// 			// std::vector<t_conf>	conf;
// 			signal(SIGINT, sighandler);
// 			if (handle_conf_file(argv[1], server.conf) == -1)
// 				return (1);
			
// 			// E/A  A RAJOUTER DANS PARSING HOST : si fichier conf 127.000.000.001 => doit devenir 127.0.0.1 : PS fonction adapt_host ci dessus
// 			// E/A  A RAJOUTER DANS PARSING => VOIR AVEC ORLANDO : QU EST CE QU ON FAIT SI HOST N EXISTE PAS? ON LE DEFINIT PAR DEFAULT OU ON RENVOIE UNE ERREUR?
// 			// A RAJOUTER DANS PARSING : directive autoindex et directive return dans bloc server 
			
// 			for (std::vector<t_conf>::iterator it =server.conf.begin(); it != server.conf.end(); it++) {
// 				if (it->host.empty())
// 					it->host = "0.0.0.0";
// 				try {
// 					it->limit_body_size = ft_stoi(it->max_body_size) ;
// 				}
// 				catch (std::exception const & e) {
// 					throw (ConfFileException("Error: body size isn't a number"));
// 				}
// 			}

// 			// SI PAS DE SERVER NAME OU SI SERVER NAME VIDE ==== > LEAKS SI RETURN(-1) ICI
// 			std::cout << "******************************** END PARSING ********************************* " << std::endl;
// 			/* ********** A EFFACER ************ */
// 			std::cout << "body_size = " << server.conf[0].max_body_size << std::endl;
// 			server.conf[0].autoindex = "on";
// 			// server.conf[1].autoindex = "off";
			
// 			/* ********************************* */
			
// 			// return (1);
			
// 			// std::vector<Listen> server_fd;
			
// 			/* Il va falloir qu on sache pour chaque fd quel serveur */
// 			server.open_listen_socket();  // NO LEAKS MEMMORY + FD OK
// 			server.create_fds(); //tableau de struct
// 			// launch_server(server.fds, server.server_fd, server.server_fd.size() + MAX_CONNECTION, server.conf);
// 			server.launch_server(server.server_fd.size() + MAX_CONNECTION);
// 			return (0);
// 		}
// 	}
// 	catch (std::bad_alloc const & e) {
// 		std::cerr << e.what() << std::endl;
// 		return (1);
// 	}
// 	catch (std::length_error const & e) {
// 		std::cerr << e.what() << std::endl;
// 		return (1);
// 	}
// 	catch (std::out_of_range const & e) {
// 		std::cerr << e.what() << std::endl;
// 		return (1);
// 	}
// 	catch (std::exception const & e) {
// 		std::string err = e.what();
// 		if (err == "exit")
// 			return (130);
// 		else
// 			std::cerr << e.what() << std::endl;
// 		return (1);
// 	}
//     std::cerr << "Error: Missing configuration file: ./webserv [configuration file]\n";
// 	return (1);
// }