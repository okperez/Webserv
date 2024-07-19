/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:43:55 by galambey          #+#    #+#             */
/*   Updated: 2024/07/19 09:33:12 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Server::Server() {
	fds = NULL;
}

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
// NO LEAKS MEMMORY + FD OK ===> DANS BLOC BEFORE LAUNCHING

/*
Si une socket ecoute deja sur le port, retourne 1 pour continuer et ne pas creer la socket
*/
bool	Server::check_port_binding(std::vector<Listen> &server_fd, std::string &port, std::string &host, int i) {
	for(std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
		if(it->getPort() == port && (it->getHost() == host || it->getHost() == "0.0.0.0")) {
			it->addIconf(i);
			return (true);
		}
	}
	return (false);
}

/* Assigning a transport address to the socket */
void	Server::bind_socket(int new_socket, struct sockaddr_in &server_addr, int port, struct addrinfo *res) {
	
	if (bind(new_socket, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
		std::cerr << "Failed to bind to port " << port;
		error_bfr_launch(new_socket, res, "");
	}
}

/*
The listen system call tells a socket that it should be capable of accepting incoming connections:
backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
*/
void	Server::listen_socket(int new_socket, int port, struct addrinfo *res) {
	
	int connection_backlog = 10; // VOIR AVEC ORLANDO POURQUOI 10? A MODIFIER?
	
	if (listen(new_socket, connection_backlog) != 0) {
		std::cerr << "Failed to listen to port " << port;
		error_bfr_launch(new_socket, res, "");
	}
	/* To set up non blocking listen socket */
	int flags = fcntl(new_socket, F_GETFL, 0); // A VERIFIER POUR FNCTL SUJET + ORLANDO
	if (flags == -1)
		error_bfr_launch(new_socket, res, "Failed to fcntl");
	if (fcntl(new_socket, F_SETFL, flags | O_NONBLOCK) == -1)
		error_bfr_launch(new_socket, res, "Failed to fcntl");
}

struct addrinfo	*Server::get_addr_info(char *data, int new_socket) {
	struct addrinfo hints, *res = NULL;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;          			// address family
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(data, NULL, &hints, &res) != 0)	// => Permet de lier l'adresse IP de l hote au port et a la socket via la structure sockaddr_in
		error_bfr_launch(new_socket, res, "Non valid host");
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
	
	for(std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
		for(std::vector<std::string>::iterator jt = it->ipv4_port.begin(); jt != it->ipv4_port.end(); jt++) {
			int port;
			std::istringstream iss(*jt);
			iss >> port;
			if (check_port_binding(server_fd, *jt, it->host, i))
				continue;
			int new_socket = -1;
			new_socket = socket(AF_INET, SOCK_STREAM, 0);   //socket set up for listening is used only for accepting connections, not for exchanging data
			if (new_socket < 0)
				error_bfr_launch(new_socket, NULL, "Failed to create server socket");
			if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &opt, 4))
				error_bfr_launch(new_socket, NULL, "Failed to create server socket");
			struct addrinfo *res = get_addr_info((char *)(it->host.data()), new_socket);
			struct sockaddr_in *server = (struct sockaddr_in *)res->ai_addr;
			if (it->host == "0.0.0.0")
				server->sin_addr.s_addr = INADDR_ANY;
			server->sin_port = htons(port);         			//The port number (the transport address)
			this->bind_socket(new_socket, *server, port, res);
			this->listen_socket(new_socket, port, res);
			Listen nw(new_socket, *jt, server->sin_addr.s_addr, it->host, i);
			nw.printlisten(); // A EFFACER
			freeaddrinfo(res);
			res = NULL;
			try {
				server_fd.push_back(nw); }
			catch (std::bad_alloc const & e) {
				close(new_socket);
				throw ; }
			catch (std::length_error const & e) {
				close(new_socket);
				throw ; }
		}
		i++;
	}
}

void	Server::create_fds() {
	
	size_t i = 0;
	
	fds = new pollfd[MAX_CONNECTION + server_fd.size()];
	// ou max de co par serveur ? 
	// fds = new pollfd[MAX_CONNECTION * server_fd.size() + server_fd.size()];
	// for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
	for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
		fds[i].fd = it->getFd();
		it->setIndex(i);
		std::cout << "CREATE_FDS POLLIN" << std::endl;
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
	
	int i = 0;
	int ret;
	
	std::cout << "A IMPLEMENTER DANS BOUCLE: CATCH EXCEPTION IF FCT CPP FAIL POUR PAS ARRETER LE SERVEUR =>> POUR EVITER BOUCLE INFINI REMPLACER VECT PAR DEQUE" << std::endl;
	std::cout << "A IMPLEMENTER : CHUNK REQUEST : " << std::endl;
	std::cout << "		- SECU OVERFLOW INT" << std::endl;
	std::cout << "		- SECU " << std::endl;
	while (1)
	{
		// Check si changement dans les fds (events/revents lies au fd(socket)) => si oui passe sinon attend
		ret = poll(fds, max_socket, 50);
		if (ret < 0) { // SI FAIL : NO LEAKS MEMORY + FD ===> TEST OK
			garbagge_server(NULL, PARENT);
			throw(ServerException("Failed to poll."));
		}
		try {
			i++;
			event_request();
		}
		catch (std::bad_alloc const & e) {
			std::cerr << e.what() << std::endl;
			continue ;
		}
		catch (std::length_error const & e) {
			std::cerr << e.what() << std::endl;
			continue ;
		}
		catch (std::out_of_range const & e) {
			std::cerr << e.what() << std::endl;
			continue ;
		}
		catch (std::exception const & e) {
			std::string err = e.what();
			if (err == "exit")
				throw ;
			else
				std::cerr << e.what() << std::endl;
			continue ;
		}
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
			for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
				if (it->getFd() == fds[i].fd) {
					new_connection(it->getFd()); // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
					return ;
				}
			}
			/* event_request sur socket listening for request ready to be handled */
			char buffer[BUFFER_SIZE] = {0};
			int n_bytes = read(fds[i].fd, buffer, BUFFER_SIZE);
			if (n_bytes < 0) // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
				handle_error_function(fds[i].fd, "500", "Fail to read", error);
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
				for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
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
				read_request(i, buffer, n_bytes); // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
			return ;
		}
		if (fds[i].revents & POLLOUT)
		{
			for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
				// if ((it->getStatus() == READING && it->getTransfer_encoding() != "chunked") || it->getStatus() == RD_TO_RESPOND) {
			// for (int j = 0; j < MAX_CONNECTION; j++) {
				// if (fds[j].fd == it->getSocket_fd()) {
					std::cout << "A IMPLEMENTER : AJOUTER TIME_OUT NOTAMMENT POUR CHUNK REQUEST TROP LONG" << std::endl;
					struct sockaddr_storage name;
					socklen_t namelen = sizeof(name);
					if (getsockname(it->getSocket_fd(), (struct sockaddr *)&name, &namelen) == -1) // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
						send_error(it, "500", "Fail getsockname", error);
					struct sockaddr_in *socket = (struct sockaddr_in *)&name;
					// A PARTIR DE LA : SI std::BAD_ALLOC RETOUR DANS LAUNCH => BOUCLE INFINI : 
					
					it->setIp_socket(socket->sin_addr.s_addr);
					// if (!it->parse_first_line(socket->sin_addr.s_addr, error))
					// 	return (requests.erase(it), (void) 0);
					
					if (it->getTransfer_encoding() != "chunked")
						it->parse_body();
					// std::cout << "body = " << it->getBody() << std::endl;
					
					int i_conf = pick_server(*it);
					// std::cout << "i_conf = " << i_conf << std::endl;
					
					// it->parse_request();
					
					it->handle_request(it->getSocket_fd(), conf[i_conf], error);
					if (it->getConnection() == "close") { // =====> Header "Connection : close" dans la requete => Il faut close une fois qu on a repondu
						for (int i = 0; i < MAX_CONNECTION; i++) {
							if (fds[i].fd == it->getSocket_fd()) 
								return (requests.erase(it), close_connection(i), (void) 0);
						}				
					}
					std::cout << "EVENT_REQUEST POLLIN" << std::endl;
					fds[i].events = POLLIN;	
					return (requests.erase(it), (void) 0);
				// }
			// }
			// return (requests.erase(it), (void) 0);
		// }
		// else if (it->getStatus() == ERASE)
		// 	return (requests.erase(it), (void) 0);
	}
		}
	}
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++)
		if ((it->getStatus() == READING && it->getTransfer_encoding() != "chunked"))
			for (int j = 0; j < MAX_CONNECTION; j++)
				if (fds[j].fd == it->getSocket_fd())
					fds[j].events = POLLOUT;
	
	
	// =====> Il n 'y a pas eu d'event on check si une requete a quelque chose a repondre
	// for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
	// 	if ((it->getStatus() == READING && it->getTransfer_encoding() != "chunked") || it->getStatus() == RD_TO_RESPOND) {
	// 		for (int j = 0; j < MAX_CONNECTION; j++) {
	// 			if (fds[j].fd == it->getSocket_fd()) {
	// 				std::cout << "A IMPLEMENTER : AJOUTER TIME_OUT NOTAMMENT POUR CHUNK REQUEST TROP LONG" << std::endl;
	// 				struct sockaddr_storage name;
	// 				socklen_t namelen = sizeof(name);
	// 				if (getsockname(it->getSocket_fd(), (struct sockaddr *)&name, &namelen) == -1) // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
	// 					send_error(it, "500", "Fail getsockname", error);
	// 				struct sockaddr_in *socket = (struct sockaddr_in *)&name;
	// 				// A PARTIR DE LA : SI std::BAD_ALLOC RETOUR DANS LAUNCH => BOUCLE INFINI : 
					
	// 				it->setIp_socket(socket->sin_addr.s_addr);
	// 				// if (!it->parse_first_line(socket->sin_addr.s_addr, error))
	// 				// 	return (requests.erase(it), (void) 0);
					
	// 				if (it->getTransfer_encoding() != "chunked")
	// 					it->parse_body();
	// 				std::cout << "body = " << it->getBody() << std::endl;
					
	// 				int i_conf = pick_server(*it);
	// 				std::cout << "i_conf = " << i_conf << std::endl;
					
	// 				// it->parse_request();
					
	// 				it->handle_request(it->getSocket_fd(), conf[i_conf], error);
	// 				if (it->getConnection() == "close") { // =====> Header "Connection : close" dans la requete => Il faut close une fois qu on a repondu
	// 					for (int i = 0; i < MAX_CONNECTION; i++) {
	// 						if (fds[i].fd == it->getSocket_fd()) 
	// 							return (requests.erase(it), close_connection(i), (void) 0);
	// 					}				
	// 				}
	// 				return (requests.erase(it), (void) 0);
	// 			}
	// 		}
	// 		return (requests.erase(it), (void) 0);
	// 	}
	// 	else if (it->getStatus() == ERASE)
	// 		return (requests.erase(it), (void) 0);
	// }
}

/* ************************************************************************* */
/* *************************** HANDLE CONNECTION *************************** */
/* ************************************************************************* */

// NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE ====> BLOC HANDLE CONNECTION 

void	Server::new_connection(int server_fd) {
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	
	std::cout << "Client asked for connection." << std::endl;
	// A VERIF SI TOUJOURS LE CAS OU NON : 
		// he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
		// By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
	int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
	if (socket_fd < 0)
		throw (ServerException("Client failed to connect."));
	for (int i = 1; i < MAX_CONNECTION; i++) {
		if (fds[i].fd == -1) {
			fds[i].fd = socket_fd;
			std::cout << "NEW_CONNECTION POLLIN" << std::endl;
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

/* Check si on a un unnique match socket_ip - socket_ip_server */
int	Server::unique_match(std::string &port, std::string &socket_ip, std::vector<int> & tmp) {
	for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
		if (port == it->getPort()) {
			if (socket_ip == it->getHost() || (it->getHost() == "localhost" && socket_ip == "127.0.0.1")) {
				if (it->getIconf().size() == 1)
					return (it->getIconf()[0]); // =====> Host a ete trouve : Il s'agit d'un server_host
				else
					tmp = it->getIconf();
			}
			else if (tmp.size() == 0 && it->getHost() == "0.0.0.0")
				tmp = it->getIconf();
		}
	}
	return (-1);
}

/* Retourne l index du serveur match */
int	Server::is_host(std::string host, std::string port, std::string socket_ip) {
	int i;
	std::vector<int> match, exact_host, server_name, default_host;

	i = unique_match(port, socket_ip, match);
	// std::cout << "i = " << i << std::endl;
	if (i > -1)
		return (i);
	for (std::vector<int>::iterator it = match.begin(); it != match.end(); it++) {
		if (conf[*it].host == socket_ip)
			exact_host.push_back(*it);
		if (conf[*it].host == "0.0.0.0")
			default_host.push_back(*it);
	}
	if (exact_host.size() == 1)
		return (exact_host[0]);
	if (exact_host.size() > 0) {
		for (std::vector<int>::iterator it = exact_host.begin(); it != exact_host.end(); it++)
			if (conf[*it].server_name == host)
				server_name.push_back(*it);
	}
	if (exact_host.size() == 0 || server_name.size() == 0) {
		for (std::vector<int>::iterator it = default_host.begin(); it != default_host.end(); it++)
			if (conf[*it].server_name == host)
				server_name.push_back(*it);
	}
	if (server_name.size() == 1)
		return (server_name[0]);
	else if (exact_host.size() > 0)
		return (exact_host[0]);
	return (default_host[0]);
}

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
	if (conf.size() == 0)
		return (0);
	return (is_host(request.getHost(), request.getPort(), request.getSocket_ip()));
}

void	Server::body_request_present(Request &request, int read, int i) {
	
	if (request.body_present()) {
		if (request.getStatus() == NEW && !request.parse_header())
			std::cout << "A IMPLEMENTER => error a deja fill response => envoyer la rep + effacer requete ";
		if (read < BUFFER_SIZE && request.getTransfer_encoding() != "chunked") {
			std::cout << "BODY_PRESENT POLLOUT" << std::endl;
			fds[i].events = POLLOUT;
			// request.setStatus(RD_TO_RESPOND);
		}
		// else if (request.getTransfer_encoding() == "chunked" /* && dans body 0 */)
		// 	request.setStatus(RD_TO_RESPOND);
		else
			request.setStatus(READING);
	}
}

/*
The normal procedure for parsing an HTTP message is to read the start-line
into a structure, read each header field line into a hash table by field name
until the empty line, and then use the parsed data to determine if a message
body is expected. If a message body has been indicated, then it is read as a
stream until an amount of octets equal to the message body length is read or the
connection is closed.
*/
void	Server::read_request(int i, char *buffer, int read) {
	
	// for (int j = 0; j < BUFFER_SIZE; j++)
	// 	std::cout << buffer[j];
	// std::cout << std::endl;

	// Si une requete a deja ete cree : 
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
		if (it->getSocket_fd() == fds[i].fd) {
			if (it->getStatus() == READING) { // UTILE ICI ?
				if (it->getTransfer_encoding() != "chunked") {
					it->addSave_buffer(buffer);
					if (read < BUFFER_SIZE) {
						// it->setStatus(RD_TO_RESPOND);
						std::cout << "READ_REQUEST 1 POLLOUT" << std::endl;
						fds[i].events = POLLOUT;
					}
				}
				else if (it->getTransfer_encoding() == "chunked" /* && dans body 0 */) {
					std::string tmp = buffer;
					int chunk = it->extract_chunked_body(tmp);
					if (chunk == 0) {
						// it->setStatus(RD_TO_RESPOND);
						std::cout << "READ_REQUEST 2 POLLOUT" << std::endl;
						fds[i].events = POLLOUT;
					}
				}
				return ;
			}
			else if (it->getStatus() == NEW) {
				body_request_present(*it, read, i);
			}
		}
	}
	// Si pas de requete correspondant a l event, creation d'i=une nouvelle requete :
	Request 	request(buffer, /* read, */ fds[i].fd, this, &this->error, &this->auth_media); // Attention , ne pas creer de request a chaque fois , il reste peut etre a lire ou il faut ecrire
	body_request_present(request, read, i);
	// if (request.body_present()) {
	// 	request.parse_request();
	// 	if (read < BUFFER_SIZE && request.getTransfer_encoding() != "chunked")
	// 		request.setStatus(RD_TO_RESPOND);
	// 	else if (request.getTransfer_encoding() != "chunked" /* && dans body 0 */)
	// 		request.setStatus(RD_TO_RESPOND);
	// 	else
	// 		request.setStatus(READING);
	// }
	requests.push_back(request);
	// std::cout << "requests[0].getTransfer_encoding() = |" << requests[0].getTransfer_encoding() << "|" << std::endl;
}

/* ************************************************************************* */
/* ********************************* ERROR ********************************* */
/* ************************************************************************* */

void	Server::send_error(std::vector<Request>::iterator it, std::string const &code, const char *mess, ErrorPages &error) {
	
	it->fill_error(code, error);
	requests.erase(it);
	throw (ServerException(mess));
}

void	Server::handle_error_function(int socket, std::string const &code, const char *mess, ErrorPages &error) {
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
		if (socket == it->getSocket_fd())
			send_error(it, code, mess, error);
	}
	Request tmp("", /* -1, */ socket, this, &error, &auth_media);
	tmp.fill_error(code, error);
	throw (ServerException(mess));
}

/* ************************************************************************* */
/* ********************************* CLOSE ********************************* */
/* ************************************************************************* */	

void	Server::error_bfr_launch(int new_socket, struct addrinfo *res, const char *s) {
	if (new_socket > -1)
		close(new_socket);
	if (res)
		freeaddrinfo(res);
	for(std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
		it->close_fd();
	throw(ServerException(s));
}

void	Server::error_bfr_launch() {
	for(std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++)
		it->close_fd();
}

/* Si control C stop listen + close socket listen */
void	Server::stop_listen() {
	for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end() ; it++) {
		fds[it->getIndex()].events = 0;
		close(fds[it->getIndex()].fd); // A VOIR SI CA MARCHE ICI
		fds[it->getIndex()].fd =  -1;
	}
}

void	Server::close_requests(int &socket) {
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
		if (socket == it->getSocket_fd()) {
			it->handle_pending_requests(error, socket);
			return ;
		}
	}
	// Request tmp(NULL, 0, socket, this, &this->auth_media);
	// tmp.handle_pending_requests(error, socket);
} 

void	Server::handle_pending_requests() {
	
	if (fds) {
		for (int i = 0; i < MAX_CONNECTION; i++) {
			if (fds[i].fd > -1) {
				close_requests(fds[i].fd);
				close (fds[i].fd);
			}
		}
		delete[] fds;
	}
}

void	Server::close_child_sockets() {
	
	if (fds) {
		for (int i = 0; i < MAX_CONNECTION; i++) {
			if (fds[i].fd > -1) {
				close (fds[i].fd);
				fds[i].fd = -1;
			}
		}
	}
}
