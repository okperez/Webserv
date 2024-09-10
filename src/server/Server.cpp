/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:43:55 by galambey          #+#    #+#             */
/*   Updated: 2024/09/10 11:37:25 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Server::Server() {
	_fds = NULL;
	_ctrlC = false;
	_time_sigint = false;
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

	struct pollfd *	Server::getFds() const {
		
		return (_fds);
	}
	
	size_t	Server::getServer_fd_size() const {
		
		return (_server_fd.size());
	}

	void	Server::setCtrlC()
	{
		_ctrlC = true;
	}

	bool	Server::getTimeSigint() const {
		
		return (_time_sigint);
	}

	void	Server::setTimeSigint()
	{
		_time_sigint = true;
	}


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
bool	Server::check_port_binding(std::string const &port, std::string const &host, int const i) {
	for(std::vector<Listen>::iterator it = _server_fd.begin(); it != _server_fd.end(); it++) {
		if(it->getPort() == port && (it->getHost() == host || it->getHost() == "0.0.0.0")) {
			it->addIconf(i);
			return (true);
		}
	}
	return (false);
}

/* Assigning a transport address to the socket */
void	Server::bind_socket(int const new_socket, struct sockaddr_in &server_addr, int const port, struct addrinfo *res) {
	
	if (bind(new_socket, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
		std::cerr << "Failed to bind to port " << port;
		error_bfr_launch(new_socket, res, "");
	}
}

/*
The listen system call tells a socket that it should be capable of accepting incoming connections:
backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
*/
void	Server::listen_socket(int const new_socket, int const port, struct addrinfo *res) {
	
	int connection_backlog = 10; // VOIR AVEC ORLANDO POURQUOI 10? A MODIFIER?
	
	if (listen(new_socket, connection_backlog) != 0) {
		std::cerr << "Failed to listen to port " << port;
		error_bfr_launch(new_socket, res, "");
	}
	/* To set up non blocking listen socket */
	if (fcntl(new_socket, F_SETFL, O_NONBLOCK) == -1)
		error_bfr_launch(new_socket, res, "Failed to fcntl");
}

struct addrinfo	*Server::get_addr_info(const char *data, int const new_socket) {
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
			if (check_port_binding(*jt, it->host, i))
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
			freeaddrinfo(res);
			res = NULL;
			try {
				_server_fd.push_back(nw); }
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
	
	_fds = new pollfd[MAX_CONNECTION + _server_fd.size()];
	for (std::vector<Listen>::iterator it = _server_fd.begin(); it != _server_fd.end(); it++) {
		_fds[i].fd = it->getFd();
		it->setIndex(i);
		_fds[i].events = POLLIN; // to set up the listen socket, ready to listen for new request from clients
		i++;
	}
	while( i < MAX_CONNECTION + _server_fd.size()) {
		_fds[i].fd = -1;
		_fds[i].events = 0;
		_fds[i].revents = 0;
		i++;
	}
}

/* ************************************************************************* */
/* ******************************* LAUNCHING ******************************* */
/* ************************************************************************* */

void	Server::launch_server(int const max_socket) {
	
	int ret;
	
	try {
		while (1)
		{
			// Check si changement dans les fds (events/revents lies au fd(socket)) => si oui passe sinon attend le temps du timeout
			ret = poll(_fds, max_socket, 50);
			if (_ctrlC) {
				close_child_sockets();
				del_all();
				throw(ServerException("exit"));
			}
			if (ret < 0) {
				garbagge_server(NULL, PARENT);
				throw(ServerException("Failed to poll."));
			}
			try {
				event_request(max_socket - 1);
				new_connection(max_socket - 1);
			}
			catch (std::exception const & e) {
				std::string err = e.what();
				if (err == "exit")
					throw;
				else if (!err.empty())
					std::cerr << e.what() << std::endl;
				continue ;
			}
		}
	}
	catch (std::exception const & e) {
		std::string err = e.what();
		if (!err.empty())
			std::cerr << e.what() << std::endl;
		stop_listen(); // BLOQUE LES DEMANDE DE CONNECTION
		handle_pending_requests(); // Envoie erreur 503 a toutes les connections acceptees ou requests en cours
		del_all();
		throw;
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
void	Server::read_request(int const i, char const *buffer, int const read) {
	
	// Si une requete a deja ete cree : 
	for (std::vector<Request>::iterator it = _requests.begin(); it != _requests.end(); it++) {
			if (it->getSocket_fd() == _fds[i].fd) {
				time_t now;
				time(&now);
				it->setT_creation(now);
				if (it->getStatus() == READING) {
					if (it->getTransfer_encoding() != "chunked" && it->getContentType() != "multipart/form-data") {
						it->addSave_buffer(buffer, read);
						if (read < BUFFER_SIZE) {
							_fds[i].events = POLLOUT;
						}
					}
					else if (it->getContentType() == "multipart/form-data") {
						std::string tmp;
						tmp.append(buffer, read);
						it->parse_upload_body(tmp);
						if (it->getStatus() == RD_TO_RESPOND)
							_fds[i].events = POLLOUT;
					}
					else if (it->getTransfer_encoding() == "chunked") {
						std::string tmp = buffer;
						try {
							int chunk = it->extract_chunked_body(tmp);
							if (chunk == 0)
								_fds[i].events = POLLOUT;
						}
						catch (std::exception const &e) {
							std::string err = e.what();
							if (err == "exit")
								throw ;
							_fds[i].events = POLLOUT;
							it->fill_significant_error("400", _error);
							throw (ServerException(""));
						}
					}
				}
				else if (it->getStatus() == NEW) {
					
					it->addSave_buffer(buffer, read);
					body_request_present(*it, read, i);
					if (it->getContentType() == "multipart/form-data" && it->getStatus() == RD_TO_RESPOND)
						_fds[i].events = POLLOUT;
				}
				return ;
			}
	}
	Request 	request(buffer, read, _fds[i].fd, this, &_error, &_auth_media);
	body_request_present(request, read, i);
	_requests.push_back(request);
}

bool	Server::request_response(int const i) {
	
	for (std::vector<Request>::iterator it = _requests.begin(); it != _requests.end(); it++) {
			if (it->getSocket_fd() == _fds[i].fd) {
				if (it->getStatus() == ERROR) {
					it->send_response(it->getSocket_fd());
					_fds[i].events = POLLIN;	
					return (close_and_erase(it), true);
				}
				if (it->getStatus() == CLOSE) {
					_fds[i].events = POLLIN;
					return (close_and_erase(it), true);
				}
				if (it->getStatus() == RD_TO_SEND) {
					it->send_response(it->getSocket_fd());
					if (it->getConnection() == "close") {
						return (it->setStatus(CLOSE), true);
					}
					_fds[i].events = POLLIN;
					return (_requests.erase(it), true);
				}
				// construction reponse
				struct sockaddr_storage name;
				socklen_t namelen = sizeof(name);
				if (getsockname(it->getSocket_fd(), (struct sockaddr *)&name, &namelen) == -1)
					send_error(it, "500", "Fail getsockname", _error);
				struct sockaddr_in *socket = (struct sockaddr_in *)&name;
				it->setIp_socket(socket->sin_addr.s_addr);
				if (it->getTransfer_encoding() != "chunked" && it->getContentType() != "multipart/form-data")
					it->parse_body();
				int i_conf = pick_server(*it);
				it->handle_request(conf[i_conf], _error);	
				return (true);
			}
		}
	return (false);
}

void	Server::no_event_request(size_t const max_socket) {
	
	for (std::vector<Request>::iterator it = _requests.begin(); it != _requests.end(); it++) {
		// Cas ou read a lu la fin de la request et etait = taille du buffer
		if ((it->getStatus() == READING && it->getTransfer_encoding() != "chunked")) {
			for (size_t i = 0; i <= max_socket; i++)
				if (_fds[i].fd == it->getSocket_fd())
					_fds[i].events = POLLOUT;
			continue;
		}
		// pour check time out request et eviter eternal pending request
		time_t now;
		time(&now);
		if (difftime(now, mktime(it->getT_creation())) > 10) {
			it->fill_error("408", _error);
			try {
				it->send_response(it->getSocket_fd());
			}
			catch (std::exception const & e) {
				if (strcmp(e.what(), "Fail to write") || strcmp(e.what(), "exit"))
					throw;
			}
			return (close_and_erase(it), (void) 0);
		}
	}
}

void	Server::event_request(size_t const max_socket) {
	
	static size_t i = _server_fd.size();
	static size_t e = max_socket;
	
	while (1)
	{
		if (_fds[i].revents & POLLIN)
		{	
			char buffer[BUFFER_SIZE] = {0};
			int n_bytes = read(_fds[i].fd, buffer, BUFFER_SIZE - 1);
			if (n_bytes < 0) {
				e = i;
				i = (i < max_socket) * (i + 1) + (i >= max_socket) * (_server_fd.size());
				handle_error_function(e);
				throw (ServerException(""));
			}
			if (!n_bytes) { // =====> LE CLIENT A INTERROMPU LA CONNECTION = (event) + (read == 0)
				for (std::vector<Request>::iterator it = _requests.begin(); it != _requests.end(); it++) {
					if (it->getSocket_fd() == _fds[i].fd) {
						_requests.erase(it);
						break;			
					}
				}
				close_connection(i);
			}
			else
				read_request(i, buffer, n_bytes); 
			e = i;
			i = (i < max_socket) * (i + 1) + (i >= max_socket) * (_server_fd.size());
			return ;
		}
		else if (_fds[i].revents & POLLOUT)
		{
			/* au lieu de send mettre status puis ici condition si status pour send envoyer reponse*/
			if (request_response(i)) {
				e = i;
				i = (i < max_socket) * (i + 1) + (i >= max_socket) * (_server_fd.size());
				return;
			}
		}
		if (i == e)
			break;
		if (i >= max_socket) {
			i = _server_fd.size();
			continue;
		}
		i++;
	}
	i = _server_fd.size();
	e = max_socket;
	no_event_request(max_socket);
}



/* ************************************************************************* */
/* *************************** HANDLE CONNECTION *************************** */
/* ************************************************************************* */

/* Called if there something to be read and handled in one of the fds */
void	Server::new_connection(size_t const max_socket) {
	
	for (size_t j = 0; j < _server_fd.size(); j++) {
		if (_fds[j].revents & POLLIN)
		{	
			/* event_request sur socket listening for clients */
			for (std::vector<Listen>::iterator it = _server_fd.begin(); it != _server_fd.end(); it++) {
				if (it->getFd() == _fds[j].fd) {
					if (new_connection(it->getFd(), max_socket))
						break ;
					// Si max co atteint une fois : j = _server_fd.size() x quitter les deux boucles
					j = _server_fd.size();
					break;
				}
			}
		}
	}
}

bool	Server::new_connection(int const server_fd, int const max_socket) {
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	
	for (int i = _server_fd.size(); i <= max_socket; i++) {
		if (_fds[i].fd < 0) {
			_fds[i].fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
			if (_fds[i].fd < 0)
				throw (ServerException("Client failed to connect."));
			if (fcntl(_fds[i].fd, F_SETFL, O_NONBLOCK) == -1) { // To set the socket non blocking
				close_connection(i);
				throw (ServerException("Client failed to connect."));
			}
			_fds[i].events = POLLIN;
			std::cout << "Client connected on socket " << _fds[i].fd << "." << std::endl;
			return (true);
		}
	}
	std::cerr << "Maximum of connection reached." << std::endl;
	return (false);
}

void	Server::close_connection(int const i) {

	std::cout << "Connection on socket " << _fds[i].fd << " closed." << std::endl;
	if (_fds[i].fd > -1)
		close(_fds[i].fd);
	_fds[i].fd = -1;
	_fds[i].revents = 0;
	_fds[i].events = 0;
}

/* ************************************************************************* */
/* ******************************** REQUEST ******************************** */
/* ************************************************************************* */

/* Check si on a un unnique match socket_ip - socket_ip_server */
int	Server::unique_match(std::string const &port, std::string const &socket_ip, std::vector<int> & tmp) {
	for (std::vector<Listen>::iterator it = _server_fd.begin(); it != _server_fd.end(); it++) {
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
int	Server::is_host(std::string const &host, std::string const &port, std::string const &socket_ip) {
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
				if (err == "exit")
					throw;
				_fds[i].events = POLLOUT;
				request.fill_significant_error(err, _error);
				_requests.push_back(request);
				throw (ServerException(""));
			}
		}
		if (read < BUFFER_SIZE && request.getTransfer_encoding() != "chunked" && request.getContentType() != "multipart/form-data")
			_fds[i].events = POLLOUT;
		else if ((request.getTransfer_encoding() == "chunked" || request.getContentType() != "multipart/form-data") && request.getStatus() == RD_TO_RESPOND)
			_fds[i].events = POLLOUT;
		else
			request.setStatus(READING);
	}
}

/* ************************************************************************* */
/* ********************************* ERROR ********************************* */
/* ************************************************************************* */

void	Server::send_error(std::vector<Request>::iterator &it, std::string const &code, const char *mess, ErrorPages &error) {
	
	it->fill_error(code, error);
	it->send_response(it->getSocket_fd());
	close_and_erase(it);
	throw (ServerException(mess));
}

void	Server::handle_error_function(int const i) {
	for (std::vector<Request>::iterator it = _requests.begin(); it != _requests.end(); it++) {
		if (_fds[i].fd == it->getSocket_fd()) {
			it->setStatus(CLOSE);
			_fds[i].events = POLLOUT;
			return;
		}
	}
	for (std::vector<Listen>::iterator it = _server_fd.begin(); it != _server_fd.end(); it++) {
		if (_fds[i].fd == it->getFd())
			return;
	}
	close_connection(i);
}

/* ************************************************************************* */
/* ********************************* CLOSE ********************************* */
/* ************************************************************************* */	

void	Server::del_all() {
	
	_error.del_all();
	_auth_media.getTypes().clear();
	std::vector<t_conf>().swap(conf); 
	std::vector<Listen>().swap(_server_fd);
	std::vector<Request>().swap(_requests);
}

void	Server::error_bfr_launch(int const new_socket, struct addrinfo *res, const char *s) {
	if (new_socket > -1)
		close(new_socket);
	if (res)
		freeaddrinfo(res);
	for(std::vector<Listen>::iterator it = _server_fd.begin(); it != _server_fd.end(); it++)
		it->close_fd();
	throw(ServerException(s));
}

void	Server::error_bfr_launch() {
	for(std::vector<Listen>::iterator it = _server_fd.begin(); it != _server_fd.end(); it++)
		it->close_fd();
}

void	Server::close_and_erase(std::vector<Request>::iterator &it) {
	for (size_t i = 0; i < MAX_CONNECTION + _server_fd.size(); i++) {
		if (_fds[i].fd == it->getSocket_fd())
			return (_requests.erase(it), close_connection(i), (void) 0);
	}
}

/* Si control C stop listen + close socket listen */
void	Server::stop_listen() {
	for (std::vector<Listen>::iterator it = _server_fd.begin(); it != _server_fd.end() ; it++) {
		_fds[it->getIndex()].events = 0;
		close(_fds[it->getIndex()].fd); // A VOIR SI CA MARCHE ICI
		_fds[it->getIndex()].fd =  -1;
	}
}

void	Server::handle_pending_requests_in_vect(int const &socket) {
	
	for (std::vector<Request>::iterator it = _requests.begin(); it != _requests.end(); it++) {
		if (socket == it->getSocket_fd()) {
			it->handle_pending_requests(_error, socket);
			return;
		}
	}
}


void	Server::handle_pending_requests() {
	
	if (_fds) {
		for (size_t i = 0; i < MAX_CONNECTION + _server_fd.size(); i++) {
			if (_fds[i].fd > -1)
				handle_pending_requests_in_vect(_fds[i].fd);
		}
		for (size_t i = 0; i < MAX_CONNECTION + _server_fd.size(); i++) {
			if (_fds[i].fd > -1) {
				close (_fds[i].fd);
				_fds[i].fd = -1;
			}
		}
		delete[] _fds;
	}
}

void	Server::close_child_sockets() {
	
	if (_fds) {
		for (size_t i = 0; i < MAX_CONNECTION + _server_fd.size(); i++) {
			if (_fds[i].fd > -1) {
				close (_fds[i].fd);
				_fds[i].fd = -1;
			}
		}
	}
}
