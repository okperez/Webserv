/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:23:55 by operez            #+#    #+#             */
/*   Updated: 2024/06/28 18:17:39 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

std::string	int_to_str(int n) {
	std::string s;
	std::ostringstream oss(s);
	
	oss << n;
	return (oss.str());
}

/* Checke si adresse ip de l host a un format valid + supprime les 0 inutiles. ex : 127.0001.0001.1 => 127.1.1.1*/
void	adapt_host(std::string &s) {
	
	int i;
	int n;
	char c;
	
	if (s == "localhost")
		return ;
	std::istringstream iss(s);
	s = "";
	for (int j = 0; j < 4; j++) {
		iss >> n;
		if (n < 0 || n > 255)
			throw (ConfFileException("Not a valid host"));
		s += int_to_str(n);
		if (j < 3) {
			iss >> c;
			if (c != '.')
				throw (ConfFileException("Not a valid host"));
			s += c;
		}
		else if (j == 3 && !iss.eof())
			throw (ConfFileException("Not a valid host"));
	}
}

int main(int argc, char **argv) 
{
	try {
		if (argc == 2)
		{
			Server server;
			// std::vector<t_conf>	conf;
			signal(SIGINT, sighandler);
			if (handle_conf_file(argv[1], server.conf) == -1)
				return (1);
			
			// A RAJOUTER DANS PARSING HOST : si fichier conf 127.000.000.001 => doit devenir 127.0.0.1 : PS fonction adapt_host ci dessus
			// A RAJOUTER DANS PARSING => VOIR AVEC ORLANDO : QU EST CE QU ON FAIT SI HOST N EXISTE PAS? ON LE DEFINIT PAR DEFAULT OU ON RENVOIE UNE ERREUR?
			for(std::vector<t_conf>::iterator it = server.conf.begin(); it != server.conf.end(); it++) {
				if (it->host.empty())
					it->host = "127.0.0.1";
			}
			std::cout << "******************************** END PARSING ********************************* " << std::endl;
			/* ********** A EFFACER ************ */

			// conf[0].server_name = "test";
			// conf[1].server_name = "test";
			// print_location(conf);
			// std::cout << "conf.root_dir " << conf[0].root_dir << std::endl;
			
			// t_conf		test;

			// test.server_name = "test";
			// conf.push_back(test);
			
			/* ********************************* */
			
			// return (1);
			
			std::vector<Listen> server_fd;
			
			/* Il va falloir qu on sache pour chaque fd quel serveur */
			server.open_listen_socket();
			server.create_fds(); //tableau de struct
			save_fds(server.fds, server.server_fd.size() + MAX_CONNECTION); // A METTRE DANS DESTRUCTEUR DE SERVEUR OU PAS A VOIR
			launch_server(server.fds, server.server_fd, server.server_fd.size() + MAX_CONNECTION, server.conf);
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