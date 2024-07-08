/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:43:55 by galambey          #+#    #+#             */
/*   Updated: 2024/07/05 17:06:21 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Server::Server() {}

Server::Server(const Server & orig) { (void) orig; }

Server::~Server() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */

/* ************************** Assignment Operator  ************************* */

Server &Server::operator=(Server & rhs) {
	(void) rhs;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */


/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

/* ************************************************************************* */
/* ***************************** BFR LAUNCHING ***************************** */
/* ************************************************************************* */

// /*
// Si une socket ecoute deja sur le port, retourne 1 pour continuer et ne pas creer la socket
// */
// bool	Server::check_port_binding(std::vector<Listen> &server_fd, std::string &port, std::string &host) {
// 	for(auto it = server_fd.begin(); it != server_fd.end(); it++) {
// 		if(it->getPort() == port && (it->getHost() == host || it->getHost() == "0.0.0.0"))
// 			return (true);
// 	}
// 	return (false);
// }

/*
Si une socket ecoute deja sur le port, retourne 1 pour continuer et ne pas creer la socket
*/
bool	Server::check_port_binding(std::vector<Listen> &server_fd, std::string &port, std::string &host, int i) {
	for(auto it = server_fd.begin(); it != server_fd.end(); it++) {
		if(it->getPort() == port && (it->getHost() == host || it->getHost() == "0.0.0.0")) {
			it->addIconf(i);
			return (true);
		}
	}
	return (false);
}

/* Assigning a transport address to the socket */
void	Server::bind_socket(int new_socket, struct sockaddr_in &server_addr, int port, std::string & host) {
	
	if (bind(new_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)))
	{
		// for(std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
		for(auto it = server_fd.begin(); it != server_fd.end(); it++)
			it->close_fd();
		std::cerr << "Failed to bind to port " << port << std::endl;
		throw(ServerException(""));
	}
}

/*
The listen system call tells a socket that it should be capable of accepting incoming connections:
backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
*/
void	Server::listen_socket(int new_socket, int port) {
	
	int connection_backlog = 10; // VOIR AVEC ORLANDO POURQUOI 10? A MODIFIER?
	if (listen(new_socket, connection_backlog) != 0) {
		// for(std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
		for(auto it = server_fd.begin(); it != server_fd.end(); it++)
			it->close_fd();
			// close (it->fd);
		std::cerr << "Failed to listen to port " << port << std::endl;
		throw(ServerException(""));
	}
	/* To set up non blocking listen socket */
	int flags = fcntl(new_socket, F_GETFL, 0); // A VERIFIER POUR FNCTL SUJET + ORLANDO
	fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);
}

struct addrinfo	*Server::get_addr_info(char *data) {
	struct addrinfo hints, *res;
	struct sockaddr_in *server;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;          			// address family
	hints.ai_socktype = SOCK_STREAM;
	int result = getaddrinfo(data, NULL, &hints, &res); // => Permet de lier l'adresse IP de l hote au port et a la socket via la structure sockaddr_in
	if (result != 0) {
		freeaddrinfo(res);
		throw(ServerException("Non valid host"));
	}
	return (res);
}

/*
Le flag SO_REUSEADDR dans setsockopt() permet d'assigner la socket a un port meme si ce dernier est en TIME_WAIT
ce qui peut arriver quand le serveur est quitte via un CTRL+C.
Comme ca permet d'autoriser la réutilisation des adresses locales, il faut faire attention
a son utillisation car cela pourrait engendrer des fqilles de securite. 
*/
void	Server::open_listen_socket() {
	
	int opt = 1;
	int i = 0;
	
	for(auto it = conf.begin(); it != conf.end(); it++) {
		for(auto jt = it->ipv4_port.begin(); jt != it->ipv4_port.end(); jt++) {
			int port;
			std::istringstream iss(*jt);
			iss >> port;
			if (check_port_binding(server_fd, *jt, it->host, i))
				continue;
			int new_socket;
			new_socket = socket(AF_INET, SOCK_STREAM, 0);   //socket set up for listening is used only for accepting connections, not for exchanging data
			if (new_socket < 0)
				throw (ServerException("Failed to create server socket")); // NO LEAKS OK
			if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &opt, 4)) {
				close(new_socket);
				throw (ServerException("Failed to create server socket")); // NO LEAKS OK
			}
			struct addrinfo *res = get_addr_info(it->host.data());
			struct sockaddr_in *server = (struct sockaddr_in *)res->ai_addr;
			if (it->host == "0.0.0.0")
				server->sin_addr.s_addr = INADDR_ANY;
			server->sin_port = htons(port);         			//The port number (the transport address)
			this->bind_socket(new_socket, *server, port, it->host); // attention LEAK SI FAILED TO BIND
			this->listen_socket(new_socket, port); // attention LEAK SI FAILED TO BIND
			Listen nw(new_socket, *jt, server->sin_addr.s_addr, it->host, i);
			nw.printlisten(); // A EFFACER
			freeaddrinfo(res);
			server_fd.push_back(nw);
		}
		i++;
	}
}

void	Server::create_fds() {
	
	int i = 0;
	
	fds = new pollfd[MAX_CONNECTION + server_fd.size()];
	// ou max de co par serveur ? 
	// fds = new pollfd[MAX_CONNECTION * server_fd.size() + server_fd.size()];
	// for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
	for (auto it = server_fd.begin(); it != server_fd.end(); it++) {
		fds[i].fd = it->getFd();
		fds[i].events = POLLIN; // to set up the listen socket, ready to listen for new request from clients
		i++;
	}
	while( i < MAX_CONNECTION + server_fd.size()) {
		fds[i].fd = -1;
		fds[i].events = 0;
		fds[i].revents = 0;
		i++;
	}
}

/* ************************************************************************* */
/* ******************************* LAUNCHING ******************************* */
/* ************************************************************************* */

void	Server::launch_server(int max_socket) {
	
	while (1)
	{
		// std::cout << "Waiting...\n";
		// Check si changement dans les fds (events/revents lies au fd(socket)) => si oui passe sinon attend
		int ret = poll(fds, max_socket, 50);
		if (ret < 0) { //verifier leak + leak de fd
			close_fds(fds, max_socket);
			throw(ServerException("Failed to poll."));
		}
		event_request();
	}
}

/* ************************************************************************* */
/* ********************************* EVENTS ******************************** */
/* ************************************************************************* */


/* Called if there something to be read and handled in one of the fds */
void	Server::event_request() {
	
	for (int i = 0; i < MAX_CONNECTION; i++)
	{
		if (fds[i].revents & POLLIN)
		{	
			/* event_request sur socket listening for clients */
			// for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
			for (auto it = server_fd.begin(); it != server_fd.end(); it++) {
				if (it->getFd() == fds[i].fd) {
					new_connection(it->getFd());
					return ;
				}
			}
			/* event_request sur socket listening for request ready to be handled */
			char buffer[BUFFER_SIZE] = {0};
			int n_bytes = read(fds[i].fd, buffer, BUFFER_SIZE); //secu si ==-1
			if (!n_bytes) { // =====> LE CLIENT A INTERROMPU LA CONNECTION = (event) + (read == 0)
			/*A significant difference between HTTP/1.1 and earlier versions of
			HTTP is that persistent connections are the default behavior of any
			HTTP connection. That is, unless otherwise indicated, the client
			SHOULD assume that the server will maintain a persistent connection,
			even after error responses from the server.

			Persistent connections provide a mechanism by which a client and a
			server can signal the close of a TCP connection. This signaling takes
			place using the Connection header field (section 14.10). Once a close
			has been signaled, the client MUST NOT send any more requests on that
			connection.
			*/
				// for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
				for (auto it = requests.begin(); it != requests.end(); it++) {
					if (it->getSocket_fd() == fds[i].fd) {
						requests.erase(it);
						break;			
					}
				}
				close_connection(i);
			}
			else
			/*
			In order to remain persistent, all messages on the connection MUST
			have a self-defined message length (i.e., one not defined by closure
			of the connection), as described in section 4.4.
			*/
				read_request(i, buffer, n_bytes);
			return ;
		}
	}
	// =====> Il n 'y a pas eu d'event on check si une requete a quelque chose a repondre
	// for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
	for (auto it = requests.begin(); it != requests.end(); it++) {
		if (it->getStatus() == READING || it->getStatus() == RD_TO_RESPOND) {
			for (int j = 0; j < MAX_CONNECTION; j++) { // A TESTER AVEC SIEGE sinon effacer les 3
				if (fds[j].fd == it->getSocket_fd()) { // A TESTER AVEC SIEGE
					struct sockaddr_storage name;
					socklen_t namelen = sizeof(name);
					getsockname(it->getSocket_fd(), (struct sockaddr *)&name, &namelen);
					struct sockaddr_in *socket = (struct sockaddr_in *)&name;
					it->parse_request(socket->sin_addr.s_addr);
					std::cout << "TEST0" << std::endl;
					int i_conf = pick_server(*it);
					// i_conf = 0;
					std::cout << "i_conf = " << i_conf << std::endl;
					it->handle_request(it->getSocket_fd(), conf[i_conf], error);
					if (it->getConnection() == "close") { // =====> Header "Connection : close" dans la requete => Il faut close une fois qu on a repondu
						for (int i = 0; i < MAX_CONNECTION; i++) {
							if (fds[i].fd == it->getSocket_fd()) 
								return (close_connection(i), (void) 0);
						}				
					}
					requests.erase(it);
					return ;
				}
			}
			requests.erase(it); // A TESTER AVEC SIEGE
			return ;
		}
	}
}

/* ************************************************************************* */
/* *************************** HANDLE CONNECTION *************************** */
/* ************************************************************************* */

void	Server::new_connection(int server_fd) {
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	
	std::cout << "Client asked for connection." << std::endl;
	// A VERIF SI TOUJOURS LE CAS OU NON : 
		// he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
		// By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
	int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
	if (socket_fd < 0) {
		std::cout << "Client failed to connect." << std::endl;
		return ;
	}
	for (int i = 1; i < MAX_CONNECTION; i++) {
		if (fds[i].fd == -1) {
			fds[i].fd = socket_fd;
			fds[i].events = POLLIN;
			std::cout << "Client connected on socket " << socket_fd << "." << std::endl;
			return ;
		}
	}
	close(socket_fd);
	std::cerr << "Maximum of connection reached." << std::endl;
	std::cerr << "Connection on socket " << socket_fd << " closed." << std::endl;

}

void	Server::close_connection(int i) {

	std::cout << "Connection on socket " << fds[i].fd << " closed." << std::endl;
	close(fds[i].fd);
	fds[i].fd = -1;
	fds[i].revents = 0;
	fds[i].events = 0;
}

/* ************************************************************************* */
/* ******************************** REQUEST ******************************** */
/* ************************************************************************* */

// /* Retourne l index du serveur correspondant au server_name */
// int	Server::is_server_name(std::string host, std::string port, in_addr_t socket_s_addr) {
// 	// // for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
// 	// for (auto it = server_fd.begin(); it != server_fd.end(); it++) {
// 	// 	if (socket_s_addr == it->getS_addr() && port == it->getPort() && conf[it->getIconf()].server_name == host)
// 	// 		return (std::cout << "it->getIconf() " << it->getIconf() << std::endl, it->getIconf());
// 	// }
// 	return (-1);
// }

// /* Retourne l index du serveur correspondant a l'host */
// int	Server::is_host(std::string host, std::string port, in_addr_t socket_s_addr, int *default_i) {
// 	int i = 0;
	
// 	std::cout << "***************" << std::endl;
// 	std::cout << "request host = " << host << std::endl;
// 	std::cout << "request port = " << port << std::endl;
// 	std::cout << "request socket_s_addr = " << socket_s_addr << std::endl << std::endl;
// 	// for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
// 	for (auto it = server_fd.begin(); it != server_fd.end(); it++) {
// 		std::cout << "server host = " << it->getHost() << std::endl;
// 		std::cout << "server port = " << it->getPort() << std::endl;
// 		std::cout << "server socket_s_addr = " << it->getS_addr() << std::endl << std::endl;
// 		if (socket_s_addr == it->getS_addr() && port == it->getPort()) {
// 			if (it->getHost() == host || (host == "localhost" && it->getHost() == "127.0.0.1"))
// 			// if (conf[it->getIconf()].host == host || (host == "localhost" && conf[it->getIconf()].host == "127.0.0.1"))
// 				return (std::cout << "it->getIconf() " << it->getIconf() << std::endl, it->getIconf());
// 			else if (*default_i == -1)
// 				*default_i = it->getIconf();
// 		}
// 	}
// 	std::cout << "***************" << std::endl;
// 	return (-1);
// }

// /* Retourne l index du serveur correspondant au server_name */
// int	Server::is_server_name(std::string host, std::string port, std::vector<int> &tmp) {
// 	for (auto it = tmp.begin(); it != tmp.end(); it++) {
// 	// for (auto it = server_fd.begin(); it != server_fd.end(); it++) {
// 		if (conf[*it].server_name == host) {
// 			for(auto jt = conf[*it].ipv4_port.begin(); jt != conf[*it].ipv4_port.end(); jt++)
// 				return (std::cout << "server_name i_conf = " << *it << std::endl, *it);
// 		}
// 	}
// 	return (-1);
// }

// /* Retourne l index du serveur match */
// int	Server::is_host(std::string &host, std::string &port, in_addr_t socket_s_addr) {
// 	int i = 0;
// 	std::vector<int> tmp;
	
	
// 	for (auto it = server_fd.begin(); it != server_fd.end(); it++) {
// 		if (port == it->getPort()) {
// 			if (socket_s_addr == it->getS_addr()) {
// 				if (it->getHost() == host || (host == "localhost" && it->getHost() == "127.0.0.1")) {
// 					if (it->getIconf().size() == 1)
// 						return (it->getIconf()[0]); // =====> Host a ete trouve : Il s'agit d'un server_host
// 					else if (it->getIconf().size() == 0)
// 						std::cout << "SI PENDANT TESTS ON RENTRE DEDANS A IMPLEMENTER SINON ALLER DANS LE ELSE " << std::endl;
// 					else
// 						tmp = it->getIconf();
// 				}
// 				else if (tmp.size() == 0)
// 					tmp = it->getIconf();
// 			}
// 			else if (it->getS_addr() == 0 && tmp.size() == 0)
// 				tmp = it->getIconf();
// 		}
// 	}
// 	// for (auto it = tmp.begin(); it != tmp.end(); it++)
// 	// 	std::cout << "vect i_conf_default = " << *it << std::endl;
// 	int j = is_server_name(host, port, tmp);  // =====> Server_name a ete trouve : Il s'agit d'un server_name existant avec le bon host et le bon port
// 	if (j != -1)
// 		return (j);
// 	int count = 0;
// 	int default_i = -1;
// 	for (auto it = tmp.begin(); it != tmp.end(); it++) {
// 		if (conf[*it].host != host) {
// 			return (*it); // =====> 1er host:port correspondant
// 		}
// 		else if (default_i == -1 && conf[*it].host == "0.0.0.0")
// 			default_i = *it;
// 	}
// 	return (default_i); // =====> 1er 0.0.0.0:port correspondant
// }

/* Retourne l index du serveur correspondant au server_name */
int	Server::is_server_name(std::string host, std::string port, std::vector<int> &tmp) {
	for (auto it = tmp.begin(); it != tmp.end(); it++) {
	// for (auto it = server_fd.begin(); it != server_fd.end(); it++) {
		if (conf[*it].server_name == host) {
			for(auto jt = conf[*it].ipv4_port.begin(); jt != conf[*it].ipv4_port.end(); jt++)
				return (std::cout << "server_name i_conf = " << *it << std::endl, *it);
		}
	}
	return (-1);
}

/* Retourne l index du serveur match */
int	Server::is_host(std::string &host, std::string &port, in_addr_t socket_s_addr, std::string socket_ip) {
	int i = 0;
	std::vector<int> tmp;
	
	// RETURN UNNIQUE MATCH OR SET TMP TO CANDIDATES
	for (auto it = server_fd.begin(); it != server_fd.end(); it++) {
		if (port == it->getPort()) {
			std::cout << "MATCH port" << std::endl;
			if (socket_ip == it->getHost() || (it->getHost() == "localhost" && socket_ip == "127.0.0.1")) {
				std::cout << "MATCH port + socket_ip" << std::endl;
				if (it->getIconf().size() == 1) {
					std::cout << "MATCH port + socket_ip + 1 candidat" << std::endl;
					return (it->getIconf()[0]); // =====> Host a ete trouve : Il s'agit d'un server_host
				}
				else
					tmp = it->getIconf();
			}
			else if (tmp.size() == 0 && it->getHost() == "0.0.0.0")
				tmp = it->getIconf();
		}
	}

	std::cout << "tmp size = " << tmp.size() << std::endl;
	for (auto it = tmp.begin(); it != tmp.end(); it++)
		std::cout << "vect i_conf_default = " << *it << std::endl;

	std::vector<int> tmp2;
	for (auto it = tmp.begin(); it != tmp.end(); it++) {
		if (conf[*it].host == socket_ip)
			tmp2.push_back(*it);
	}

	std::cout << std::endl << "tmp2 size = " << tmp2.size() << std::endl;
	for (auto it = tmp2.begin(); it != tmp2.end(); it++)
		std::cout << "vect i_conf_default = " << *it << std::endl;
	if (tmp2.size() == 1)
		return (tmp2[0]);
	std::vector<int> tmp3;
	for (auto it = tmp2.begin(); it != tmp2.end(); it++) {
		if (conf[*it].server_name == host)
			tmp3.push_back(*it);
	}
	std::cout << std::endl << "tmp3 size = " << tmp3.size() << std::endl;
	for (auto it = tmp3.begin(); it != tmp3.end(); it++)
		std::cout << "vect i_conf_default = " << *it << std::endl;
	
	if (tmp3.size() == 1)
		return (tmp3[0]);
	else if (tmp2.size() > 0)
		return (tmp2[0]);
	else
		return (tmp[0]);
}


// /*
// URI Comparison

//    When comparing two URIs to decide if they match or not, a client
//    SHOULD use a case-sensitive octet-by-octet comparison of the entire
//    URIs, with these exceptions:

//       - A port that is empty or not given is equivalent to the default
//         port for that URI-reference;

//         - Comparisons of host names MUST be case-insensitive; // OK

//         - Comparisons of scheme names MUST be case-insensitive; // OK

//         - An empty abs_path is equivalent to an abs_path of "/".

//    Characters other than those in the "reserved" and "unsafe" sets (see
//    RFC 2396 [42]) are equivalent to their ""%" HEX HEX" encoding.

//    For example, the following three URIs are equivalent:

//       http://abc.com:80/~smith/home.html
//       http://ABC.com/%7Esmith/home.html
//       http://ABC.com:/%7esmith/home.html

// */
// int	Server::pick_server(Request &request) {
// 	std::istringstream	iss(request.getHost());
// 	std::string         	host;
// 	std::string         	port;
// 	int						i_host;
// 	int						i_name;
// 	int						i_default = -1;
	
// 	std::getline(iss, host, ':');
// 	str_tolower(host);
// 	std::cout << host << std::endl;
// 	std::getline(iss, port);
// 	if (conf.size() == 0)
// 		return (0);
// 	i_host = is_host(host, port, request.getSocket_s_addr(), &i_default);
// 	if (i_host > -1) // =====> Host a ete trouve : Il s'agit d'un server_host SINON le premier server correspondant a un host:port a ete save dans i_default
// 		return (i_host);
// 	i_name = is_server_name(host, port, request.getSocket_s_addr());
// 	if (i_name > -1) // =====> Server_name a ete trouve : Il s'agit d'un server_name existant avec le bon host et le bon port
// 		return (i_name);
// 	else // =====> Pas de server_name avec port et host ok trouve => on renvoie i_default
// 		return (i_default);
// }

/*
URI Comparison

   When comparing two URIs to decide if they match or not, a client
   SHOULD use a case-sensitive octet-by-octet comparison of the entire
   URIs, with these exceptions:

      - A port that is empty or not given is equivalent to the default
        port for that URI-reference;

        - Comparisons of host names MUST be case-insensitive; // OK

        - Comparisons of scheme names MUST be case-insensitive; // OK

        - An empty abs_path is equivalent to an abs_path of "/".

   Characters other than those in the "reserved" and "unsafe" sets (see
   RFC 2396 [42]) are equivalent to their ""%" HEX HEX" encoding.

   For example, the following three URIs are equivalent:

      http://abc.com:80/~smith/home.html
      http://ABC.com/%7Esmith/home.html
      http://ABC.com:/%7esmith/home.html

*/
int	Server::pick_server(Request &request) {
	std::istringstream	iss(request.getHost());
	std::string         	host;
	std::string         	port;
	
	std::getline(iss, host, ':');
	str_tolower(host);
	std::cout << host << std::endl;
	std::getline(iss, port);
	if (conf.size() == 0)
		return (0);
	return (is_host(host, port, request.getSocket_s_addr(), request.getSocket_ip()));
}

/*
The normal procedure for parsing an HTTP message is to read the start-line
into a structure, read each header field line into a hash table by field name
until the empty line, and then use the parsed data to determine if a message
body is expected. If a message body has been indicated, then it is read as a
stream until an amount of octets equal to the message body length is read or the
connection is closed.
*/
/*
  revoir choose_server avec info de ce site :
  https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms
  */
  // est ce qu on verifie que le port est bien ecoute + meme serveur name + meme host?
void	Server::read_request(int i, char *buffer, int read) {

	int			i_conf = 0;
	
	for (int j = 0; j < BUFFER_SIZE; j++)
		std::cout << buffer[j];
	std::cout << std::endl;

	// Si une requete a deja ete cree : 
	// for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
	for (auto it = requests.begin(); it != requests.end(); it++) {
		if (it->getSocket_fd() == fds[i].fd) {
			if (it->getStatus() == READING) { // UTILE ICI ?
				it->addSave_buffer(buffer);
				if (read < BUFFER_SIZE) {
					it->setStatus(RD_TO_RESPOND);
				}
				return ;
			}
		}
	}
	// Si pas de requete correspondant a l event, creation d'i=une nouvelle requete :
	Request 	request(buffer, read, fds[i].fd); // Attention , ne pas creer de request a chaque fois , il reste peut etre a lire ou il faut ecrire
	requests.push_back(request);
}
