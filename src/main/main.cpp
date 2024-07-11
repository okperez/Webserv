/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/07/11 18:57:47 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

int main(int argc, char **argv) 
{
	try {
		if (argc == 2)
		{
			Server server;
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
			
			// return (1);
			
			// std::vector<Listen> server_fd;
			
			/* Il va falloir qu on sache pour chaque fd quel serveur */
			server.open_listen_socket();
			server.create_fds(); //tableau de struct
			// launch_server(server.fds, server.server_fd, server.server_fd.size() + MAX_CONNECTION, server.conf);
			server.launch_server(server.server_fd.size() + MAX_CONNECTION);
			return (0);
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
    std::cerr << "Error: Missing configuration file: ./webserv [configuration file]\n";
	return (1);
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <arpa/inet.h>
// #include <unistd.h>

// int main() {
//     struct addrinfo hints, *res, *p;
//     int status;
//     char ipstr[INET6_ADDRSTRLEN];

//     // Effacer la structure hints
//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_UNSPEC; // AF_INET ou AF_INET6 pour forcer version
//     hints.ai_socktype = SOCK_STREAM; // ou SOCK_DGRAM pour UDP

//     // Nom de serveur et service
//     const char *hostname = "allo.com"; // ou l'adresse IP ou le nom de domaine réel de votre serveur
//     const char *service = "8080"; // peut être "8560" selon le port que vous souhaitez utiliser

//     // Appel à getaddrinfo
//     if ((status = getaddrinfo(hostname, service, &hints, &res)) != 0) {
//         fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
//         return 1;
//     }

//     printf("Adresses IP pour %s:\n\n", hostname);

//     // Boucler à travers les résultats et les afficher
//     for(p = res; p != NULL; p = p->ai_next) {
//         void *addr;
//         std::string ipver;

//         // Obtenir le pointeur vers l'adresse, selon si IPv4 ou IPv6
//         if (p->ai_family == AF_INET) { // IPv4
//             struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
//             addr = &(ipv4->sin_addr);
//             ipver = "IPv4";
//         } else { // IPv6
//             struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
//             addr = &(ipv6->sin6_addr);
//             ipver = "IPv6";
//         }

//         // Convertir l'adresse IP en une chaîne de caractères
//         inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
//         printf("  %s: %s\n", ipver.data(), ipstr);
//     }

//     freeaddrinfo(res); // Libérer la mémoire allouée par getaddrinfo

//     return 0;
// }
