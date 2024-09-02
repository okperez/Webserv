/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:43:55 by galambey          #+#    #+#             */
/*   Updated: 2024/09/02 16:10:25 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Server::Server() {
	fds = NULL;
	std::string tmp = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: 158\r\n\r\n<!DOCTYPE html>\n<html>\n<head>\n\t<title> Internal Server Error</title>\n</head>\n<body>\n\t<h1>500</h1>\n\t<h1> Internal Server Error</h1>\n</body>\n</html>\r\n";
	_bad_alloc = false;
	_err_alloc = tmp.data();
	_rc_err_alloc = false;
	ctrlC = false;
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
	
	// if (fcntl(new_socket, F_SETFL, O_NONBLOCK) == -1)
	// 	error_bfr_launch(new_socket, res, "Failed to fcntl");
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
	for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
		fds[i].fd = it->getFd();
		it->setIndex(i);
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

// void	Server::launch_server(int max_socket) {
	
// 	int ret;
	
	
// 	std::cout << "VOIR SI ON THROW UN SERVER EXCEPTION OU REQUESTEXCEPTION EXIT DU COUP CAR PLUS BESOIN DANS HANDLING PENDING REQUEST\n" << std::endl;
// 	std::cout << "Si ctrl-c avec siege : 1 leak de fd potentiel :\n";
// 	std::cout << "		- SECU OVERFLOW INT" << std::endl;
// 	while (1)
// 	{
// 		// Check si changement dans les fds (events/revents lies au fd(socket)) => si oui passe sinon attend le temps du timeout
// 		ret = poll(fds, max_socket, 50);
// 		if (ret < 0) { // SI FAIL : NO LEAKS MEMORY + FD ===> TEST OK
// 			garbagge_server(NULL, PARENT);
// 			throw(ServerException("Failed to poll."));
// 		}
// 		try {
// 			if (_bad_alloc) {
// 				write(fds[_ind_err_alloc].fd, _err_alloc, strlen(_err_alloc));
// 				if (_rc_err_alloc)
// 					close_and_erase(_it_err_alloc);
// 				else
// 					close_connection(_ind_err_alloc);
// 				_rc_err_alloc = false;
// 				_bad_alloc = false;
// 				continue;
// 			}
// 			event_request(max_socket - 1);
// 		}
// 		catch (std::bad_alloc const & e) {
// 			std::cerr << e.what() << std::endl;
// 			_bad_alloc = true;
// 			continue ;
// 		}
// 		catch (std::length_error const & e) {
// 			std::cerr << e.what() << std::endl;
// 			_bad_alloc = true;
// 			continue ;
// 		}
// 		catch (std::out_of_range const & e) {
// 			std::cerr << e.what() << std::endl;
// 			_bad_alloc = true;
// 			continue ;
// 		}
// 		catch (std::exception const & e) {
// 			std::cout << "catch 0\n";
// 			std::string err = e.what();
// 			if (err == "exit") {
// 				std::cout << "EXIT\n";
// 				// handle_pending_requests(); // Envoie erreur 500 a toutes les connections acceptees ou requests en cours
// 				throw;
// 				exit(130) ;
// 			}
// 			else if (!err.empty())
// 				std::cerr << e.what() << std::endl;
// 			continue ;
// 		}
// 	}
// }

void	Server::launch_server(int max_socket) {
	
	int ret;
	
	
	std::cout << "VOIR SI ON THROW UN SERVER EXCEPTION OU REQUESTEXCEPTION EXIT DU COUP CAR PLUS BESOIN DANS HANDLING PENDING REQUEST\n" << std::endl;
	std::cout << "Si ctrl-c avec siege : 1 leak de fd potentiel :\n";
	std::cout << "		- SECU OVERFLOW INT" << std::endl;
	try {
	while (1)
	{
		// Check si changement dans les fds (events/revents lies au fd(socket)) => si oui passe sinon attend le temps du timeout
		ret = poll(fds, max_socket, 50);
		if (ctrlC) {
			std::cerr << "ctrlc\n";
			del_all();
			throw(ServerException("exit"));
		}
		// sleep(3);
		if (ret < 0) { // SI FAIL : NO LEAKS MEMORY + FD ===> TEST OK
			garbagge_server(NULL, PARENT);
			throw(ServerException("Failed to poll."));
		}
		try {
			// if (_bad_alloc) {
			// 	write(fds[_ind_err_alloc].fd, _err_alloc, strlen(_err_alloc));
			// 	if (_rc_err_alloc)
			// 		close_and_erase(_it_err_alloc);
			// 	else
			// 		close_connection(_ind_err_alloc);
			// 	_rc_err_alloc = false;
			// 	_bad_alloc = false;
			// 	continue;
			// }
			event_request(max_socket - 1);
		}
		// catch (std::bad_alloc const & e) {
		// 	std::cerr << "catch 0\n";
		// 	std::cerr << e.what() << std::endl;
		// 	_bad_alloc = true;
		// 	continue ;
		// }
		// catch (std::length_error const & e) {
		// 	std::cerr << "catch 1\n";
		// 	std::cerr << e.what() << std::endl;
		// 	_bad_alloc = true;
		// 	continue ;
		// }
		// catch (std::out_of_range const & e) {
		// 	std::cerr << "catch 2\n";
		// 	std::cerr << e.what() << std::endl;
		// 	_bad_alloc = true;
		// 	continue ;
		// }
		catch (std::exception const & e) {
			std::cerr << "catch 3\n";
			std::string err = e.what();
			if (err == "exit") {
				// std::cerr << "EXIT\n";
				// handle_pending_requests(); // Envoie erreur 500 a toutes les connections acceptees ou requests en cours
				throw;
				// exit(130) ;
			}
			else if (!err.empty()) {
				// std::cerr << "catch 4\n";
				std::cerr << e.what() << std::endl;
			}
			continue ;
		}
	}
	}
	catch (std::exception const & e) {
		std::cout << "catch 00\n";
		std::string err = e.what();
		if (err == "exit") {
			std::cout << "EXIT\n";
			// handle_pending_requests(); // Envoie erreur 500 a toutes les connections acceptees ou requests en cours
			stop_listen(); // BLOQUE LES DEMANDE DE CONNECTION
			handle_pending_requests(); // Envoie erreur 503 a toutes les connections acceptees ou requests en cours
			del_all();
			std::cout << "EXIT\n";
			throw;
			// exit(130) ;
		}
		else if (!err.empty()) {
			// std::cout << "catch 01\n";
			// std::cout << "ERR" << err << std::endl;
			std::cerr << e.what() << std::endl;
		}
	}
}

/* ************************************************************************* */
/* ********************************* EVENTS ******************************** */
/* ************************************************************************* */

/*
The normal procedure for parsing an HTTP message is to read the start-line
into a structure, read each header field line into a hash table by field name
until the empty line, and then use the parsed data to determine if a message
body is expected. If a message body has been indicated, then it is read as a
stream until an amount of octets equal to the message body length is read or the
connection is closed.
*/
void	Server::read_request(int i, char *buffer, int read) {
	
	// Si une requete a deja ete cree : 
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
		// try {
			if (it->getSocket_fd() == fds[i].fd) {
				time_t now;
				time(&now);
				it->setT_creation(now);
				if (it->getStatus() == READING) {
					if (it->getTransfer_encoding() != "chunked" && it->getContentType() != "multipart/form-data") {
						it->addSave_buffer(buffer, read);
						if (read < BUFFER_SIZE) {
							fds[i].events = POLLOUT;
						}
					}
					else if (it->getContentType() == "multipart/form-data") {
						std::string tmp;
						tmp.append(buffer, read);
						it->parse_upload_body(tmp);
						if (it->getStatus() == RD_TO_RESPOND)
							fds[i].events = POLLOUT;
					}
					else if (it->getTransfer_encoding() == "chunked") {
						std::string tmp = buffer;
						try {
							int chunk = it->extract_chunked_body(tmp);
							std::cout << "read_request 1 body = |" << it->getBody() << "|" << std::endl;
							if (chunk == 0)
								fds[i].events = POLLOUT;
						}
						catch (std::exception const &e) {
							std::cout << "catch 1\n";
							std::string err = e.what();
							if (err == "exit")
								throw ;
							fds[i].events = POLLOUT;
							it->fill_significant_error("400", error);
							throw (ServerException(""));
						}
					}
				}
				else if (it->getStatus() == NEW) {
					
					it->addSave_buffer(buffer, read);
					body_request_present(*it, read, i);
					std::cout << "read_request 2 body = |" << it->getBody() << "|" << std::endl;
					if (it->getContentType() == "multipart/form-data" && it->getStatus() == RD_TO_RESPOND)
						fds[i].events = POLLOUT;
				}
				return ;
			}
		// }
		// catch (std::bad_alloc const &e) { bad_alloc_error(i, &it); throw ; }
		// catch (std::length_error const &e) { bad_alloc_error(i, &it); throw ; }
		// catch (std::out_of_range const &e) { bad_alloc_error(i, &it); throw ; }
		// catch (std::exception const &e) {
		// 	std::cout << "catch 2\n";
		// 	throw ;
		// }
	}
	// sinon, creation requete :
	// try {
		Request 	request(buffer, read, fds[i].fd, this, &this->error, &this->auth_media);
		body_request_present(request, read, i);
		requests.push_back(request);
	// }
	// catch (std::bad_alloc const &e) { _ind_err_alloc = i; throw ; }
	// catch (std::length_error const &e) { _ind_err_alloc = i; throw ; }
	// catch (std::out_of_range const &e) { _ind_err_alloc = i; throw ; }
	// catch (std::exception const &e) {
	// 		std::cout << "catch 3\n";
	// 	throw ;
	// }
}

bool	Server::request_response(int i) {
	
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
		// try {
			std::cout << "Server::request_response\n";
			// envoi reponse a client
			if (it->getSocket_fd() == fds[i].fd) {
				if (it->getStatus() == ERROR /* || it->getStatus() == CLOSE */) {
					it->send_response(it->getSocket_fd());
					fds[i].events = POLLIN;	
					return (close_and_erase(it), true);
				}
				// if (it->getStatus() == CLOSE) {
				// 	fds[i].events = POLLIN;	
				// 	return (close_and_erase(it), true);
				// }
				// if (it->getStatus() == RD_TO_SEND) {
				// 	it->send_response(it->getSocket_fd());
				// 	fds[i].events = POLLIN;
				// 	if (it->getConnection() == "close")
				// 		return (close_and_erase(it), true);
				// 	return (requests.erase(it), true);
				// }
				if (it->getStatus() == CLOSE) {
					fds[i].events = POLLIN;
					return (close_and_erase(it), true);
				}
				if (it->getStatus() == RD_TO_SEND) {
					it->send_response(it->getSocket_fd());
					if (it->getConnection() == "close") {
						return (it->setStatus(CLOSE), true);
					}
					fds[i].events = POLLIN;
					return (requests.erase(it), true);
				}
				// construction reponse
				struct sockaddr_storage name;
				socklen_t namelen = sizeof(name);
				if (getsockname(it->getSocket_fd(), (struct sockaddr *)&name, &namelen) == -1) // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
					send_error(it, "500", "Fail getsockname", error);
				struct sockaddr_in *socket = (struct sockaddr_in *)&name;
				it->setIp_socket(socket->sin_addr.s_addr);
				if (it->getTransfer_encoding() != "chunked" && it->getContentType() != "multipart/form-data")
					it->parse_body();
				int i_conf = pick_server(*it);
				it->handle_request(conf[i_conf], error);	
				return (true);
			}
		}
	// 	catch (std::bad_alloc const &e) { bad_alloc_error(i, &it); throw ; }
	// 	catch (std::length_error const &e) { bad_alloc_error(i, &it); throw ; }
	// 	catch (std::out_of_range const &e) { bad_alloc_error(i, &it); throw ; }
	// 	catch (std::exception const &e) {
	// 		std::cout << "catch 4\n";
	// 		throw ;}
	// }
	return (false);
}

void	Server::no_event_request(int max_socket) {
	
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
		// Cas ou read a lu la fin de la request et etait = taille du buffer
		if ((it->getStatus() == READING && it->getTransfer_encoding() != "chunked")) {
			for (int i = 0; i <= max_socket; i++)
				if (fds[i].fd == it->getSocket_fd())
					fds[i].events = POLLOUT;
			continue;
		}
		// pour check time out request et eviter eternal pending request
		time_t now;
		time(&now);
		if (difftime(now, mktime(it->getT_creation())) > 10) {
			it->fill_error("408", error);
			try {
				it->send_response(it->getSocket_fd());
			}
			catch (std::exception const & e) {
				std::cout << "catch 4\n ATTENTION PAS DE THROW si exit";
				if (strcmp(e.what(), "Fail to write"))
					throw;
			}
			return (close_and_erase(it), (void) 0);
		}
	}
}

// /* Called if there something to be read and handled in one of the fds */
// void	Server::event_request(int max_socket) {
	
// 	static int i = 0;
// 	static int e = max_socket;
// 	// /* static  */int e = server_fd.size() + requests.size();
// 	// std::cout << "e = " << e << std::endl;
	
// 	// std::cout << "EVENT_REQUEST\n";
// 	while (1)
// 	{
// 		if (fds[i].revents & POLLOUT)
// 		{
// 			/* au lieu de send mettre status puis ici condition si status pour send envoyer reponse*/
// 			if (request_response(i)) {
// 				e = i;
// 				i = (i != max_socket) * (i + 1);
// 				return;
// 			}
// 		}
// 		else if (fds[i].revents & POLLIN)
// 		{	
// 			/* event_request sur socket listening for clients */
// 			for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
// 				if (it->getFd() == fds[i].fd) {
// 					if (new_connection(it->getFd(), max_socket)) { // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
// 						e = i;
// 						i = (i != max_socket) * (i + 1);
// 						return ;
// 					}
// 				}
// 			}
// 			/* event_request sur socket listening for request ready to be handled */
// 			char buffer[BUFFER_SIZE] = {0};
// 			int n_bytes = read(fds[i].fd, buffer, BUFFER_SIZE - 1);
// 			if (n_bytes < 0) {// NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
// 				if (i == e)
// 					break;
// 				if (i == max_socket) {
// 					i = 0;
// 					continue ;
// 				}
// 				i++;
// 				continue;
// 			}
// 				// handle_error_function(i, "500", "Fail to read", error);
// 			if (!n_bytes) { // =====> LE CLIENT A INTERROMPU LA CONNECTION = (event) + (read == 0)
// 				for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
// 					if (it->getSocket_fd() == fds[i].fd) {
// 						requests.erase(it);
// 						break;			
// 					}
// 				}
// 				close_connection(i);
// 			}
// 			else
// 				read_request(i, buffer, n_bytes); // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
// 			e = i;
// 			i = (i != max_socket) * (i + 1);
// 			return ;
// 		}
// 		if (i == e)
// 			break;
// 		if (i == max_socket/*  && i != e */) {
// 			i = 0;
// 			continue;
// 		}
// 		// if (i == e)
// 		// 	break;
// 		i++;
// 	}
// 	i = 0;
// 	e = max_socket;
// 	no_event_request(max_socket);
// }

/* Called if there something to be read and handled in one of the fds */
void	Server::event_request(int max_socket) {
	
	static int i = 0;
	static int e = max_socket;
	int d = i;
	// bool max_co = false;
	// /* static  */int e = server_fd.size() + requests.size();
	// std::cout << "e = " << e << std::endl;
	
	while (1) {
		// std::cout << "1ere boucle\n";
		if (fds[i].revents & POLLIN)
		{	
			/* event_request sur socket listening for clients */
			for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
				if (it->getFd() == fds[i].fd) {
					if (new_connection(it->getFd(), max_socket)) { // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
						// e = i;
						if (i != e)
							i = (i != max_socket) * (i + 1);
						break ;
					}
					// Si max co atteint une fois : i = e x quitter boucle while
					i = e;
					break;
				}
			}
		}
		if (i == e) {
			// i = (i != max_socket) * (i + 1);
			break;
		}
		if (i == max_socket/*  && i != e */) {
			i = 0;
			continue;
		}
		i++;
	}
	i = d;
	while (1)
	{
		// std::cout << "2eme boucle\n";
		// if (fds[i].revents & POLLOUT)
		// {
		// 	/* au lieu de send mettre status puis ici condition si status pour send envoyer reponse*/
		// 	if (request_response(i)) {
		// 		e = i;
		// 		i = (i != max_socket) * (i + 1);
		// 		return;
		// 	}
		// }
		/* else */ if (fds[i].revents & POLLIN)
		{	
			// /* event_request sur socket listening for clients */
			// for (std::vector<Listen>::iterator it = server_fd.begin(); it != server_fd.end(); it++) {
			// 	if (it->getFd() == fds[i].fd) {
			// 		if (new_connection(it->getFd(), max_socket)) { // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
			// 			e = i;
			// 			i = (i != max_socket) * (i + 1);
			// 			return ;
			// 		}
			// 	}
			// }
			/* event_request sur socket listening for request ready to be handled */
			char buffer[BUFFER_SIZE] = {0};
			int n_bytes = read(fds[i].fd, buffer, BUFFER_SIZE - 1);
			if (n_bytes < 0) {// NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
				if (i == e)
					break;
				if (i == max_socket) {
					i = 0;
					continue ;
				}
				i++;
				continue;
			}
				// handle_error_function(i, "500", "Fail to read", error);
			if (!n_bytes) { // =====> LE CLIENT A INTERROMPU LA CONNECTION = (event) + (read == 0)
				for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
					if (it->getSocket_fd() == fds[i].fd) {
						requests.erase(it);
						break;			
					}
				}
				close_connection(i);
			}
			else
				read_request(i, buffer, n_bytes); // NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE
			e = i;
			i = (i != max_socket) * (i + 1);
			return ;
		}
		else if (fds[i].revents & POLLOUT)
		{
			/* au lieu de send mettre status puis ici condition si status pour send envoyer reponse*/
			if (request_response(i)) {
				e = i;
				i = (i != max_socket) * (i + 1);
				return;
			}
		}
		if (i == e)
			break;
		if (i == max_socket/*  && i != e */) {
			i = 0;
			continue;
		}
		// if (i == e)
		// 	break;
		i++;
	}
	i = 0;
	e = max_socket;
	no_event_request(max_socket);
}



/* ************************************************************************* */
/* *************************** HANDLE CONNECTION *************************** */
/* ************************************************************************* */

// NO LEAKS MEMMORY + FD  && SI FAIL SERVEUR CONTINUE ====> BLOC HANDLE CONNECTION 

// void	Server::new_connection(int server_fd) {
// 	struct sockaddr_in client_addr;
// 	int client_addr_len = sizeof(client_addr);
	
// 	// std::cout << "Client asked for connection." << std::endl;
// 	// A VERIF SI TOUJOURS LE CAS OU NON : 
// 		// he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
// 		// By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
// 	int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
// 	if (socket_fd < 0)
// 		throw (ServerException("Client failed to connect."));
// 	for (int i = 1; i < MAX_CONNECTION; i++) {
// 		if (fds[i].fd == -1) {
// 			fds[i].fd = socket_fd;
// 			fds[i].events = POLLIN;
// 			std::cout << "Client connected on socket " << socket_fd << "." << std::endl;
// 			return ;
// 		}
// 	}
// 	close(socket_fd);
// 	std::cerr << "Maximum of connection reached." << std::endl;
// 	std::cerr << "Connection on socket " << socket_fd << " closed." << std::endl;
// }

bool	Server::new_connection(int server_fd, int max_socket) {
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	
	// std::cout << "Client asked for connection." << std::endl;
	// A VERIF SI TOUJOURS LE CAS OU NON : 
		// he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
		// By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
	// int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
	// if (socket_fd < 0)
	// 	throw (ServerException("Client failed to connect."));
	for (int i = 0; i <= max_socket; i++) {
		if (fds[i].fd < 0) {
			fds[i].fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
			if (fds[i].fd < 0)
				throw (ServerException("Client failed to connect."));
			fds[i].events = POLLIN;
			std::cout << "Client connected on socket " << fds[i].fd << "." << std::endl;
			return (true);
		}
	}
	std::cerr << "Maximum of connection reached." << std::endl;
	return (false);
}

void	Server::close_connection(int i) {

	std::cout << "Connection on socket " << fds[i].fd << " closed." << std::endl;
	if (fds[i].fd > -1)
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
		if (request.getStatus() == NEW) {
			try { 
				request.parse_request();
			}
			catch (std::exception const &e) {
				std::string err = e.what();
				std::cout << "hhh\n" ;
				std::cout << "catch 5\n";
				if (err == "exit")
					throw;
				fds[i].events = POLLOUT;
				request.fill_significant_error(err, error);
				requests.push_back(request);
				throw (ServerException(""));
			}
		}
		if (read < BUFFER_SIZE && request.getTransfer_encoding() != "chunked" && request.getContentType() != "multipart/form-data")
			fds[i].events = POLLOUT;
		else if ((request.getTransfer_encoding() == "chunked" || request.getContentType() != "multipart/form-data") && request.getStatus() == RD_TO_RESPOND)
			fds[i].events = POLLOUT;
		else
			request.setStatus(READING);
	}
}

/* ************************************************************************* */
/* ********************************* ERROR ********************************* */
/* ************************************************************************* */

void	Server::send_error(std::vector<Request>::iterator it, std::string const &code, const char *mess, ErrorPages &error) {
	
	it->fill_error(code, error);
	it->send_response(it->getSocket_fd());
	close_and_erase(it);
	// requests.erase(it);
	throw (ServerException(mess));
}

void	Server::handle_error_function(int i, std::string const &code, const char *mess, ErrorPages &error) {
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
		if (fds[i].fd == it->getSocket_fd())
			send_error(it, code, mess, error);
	}
	Request tmp("", 0, fds[i].fd, this, &error, &auth_media/* , 0 */);
	tmp.fill_error(code, error);
	close_connection(i);
	throw (ServerException(mess));
}

void	Server::bad_alloc_error(int i, std::vector<Request>::iterator *it) {
	_rc_err_alloc = true;
	_ind_err_alloc = i;
	_it_err_alloc = *it;
}

/* ************************************************************************* */
/* ********************************* CLOSE ********************************* */
/* ************************************************************************* */	

void	Server::del_all() {
	
	error.del_all();
	auth_media.getTypes().clear();
	std::vector<t_conf>().swap(conf); 
	std::vector<Listen>().swap(server_fd);
	// for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++)
	// 	{
	// std::cout << "hey0" << std::endl;

	// 		it->del_all();}
	// std::cout << "hey" << std::endl;
	std::vector<Request>().swap(requests);
}

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

void	Server::close_and_erase(std::vector<Request>::iterator it) {
	for (size_t i = 0; i < MAX_CONNECTION + server_fd.size(); i++) {
		if (fds[i].fd == it->getSocket_fd())
			return (requests.erase(it), close_connection(i), (void) 0);
	}
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
}

bool	Server::handle_pending_requests_in_vect(int &socket) {
	
	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it++) {
		if (socket == it->getSocket_fd()) {
			close_requests(socket);
			close_and_erase(it);
			return (true);
		}
	}
	return (false);
}


void	Server::handle_pending_requests() {
	
	if (fds) {
		for (size_t i = 0; i < MAX_CONNECTION + server_fd.size(); i++) {
			if (fds[i].fd > -1) {
				if (!handle_pending_requests_in_vect(fds[i].fd)) {
					close_requests(fds[i].fd);
					close (fds[i].fd);
				}
			}
		}
		delete[] fds;
	}
}

void	Server::close_child_sockets() {
	
	if (fds) {
		for (size_t i = 0; i < MAX_CONNECTION + server_fd.size(); i++) {
			if (fds[i].fd > -1) {
				close (fds[i].fd);
				fds[i].fd = -1;
			}
		}
	}
}
