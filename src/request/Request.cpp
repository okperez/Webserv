/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/06/30 09:21:33 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Request::Request() {}

Request::Request(char const *buffer, int read, int socket) {
	// std::string buff = buffer;

	// A UPDATE
	socket_fd = socket;
	save_buffer = buffer;
	// response_content = "";
	// port = "";
	// body = "";
	// content_type = "";
	// lenght = 0;
	
	if (read < BUFFER_SIZE)
		status = RD_TO_RESPOND;
	else
		status = READING;
}

Request::Request(const Request & orig) : status(orig.status), save_buffer(orig.save_buffer) , socket_fd(orig.socket_fd){
	(void) orig;
}

Request::~Request() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */
/* ************************** Assignment Operator  ************************* */

Request &Request::operator=(Request const & rhs) {
	socket_fd = rhs.socket_fd;
	status = rhs.status;
	save_buffer = rhs.save_buffer;
	response_content = rhs.response_content;
	method = rhs.method;
	target = rhs.target;
	version = rhs.version;
	host = rhs.host;
	port = rhs.port;
	agent = rhs.agent;
	media = rhs.media;
	body = rhs.body;
	content_type = rhs.content_type;
	connection = rhs.connection;
	lenght = rhs.lenght;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

int Request::getStatus() const {
	return (status);
}

int Request::getSocket_fd() const {
	return (socket_fd);
}

std::string Request::getHost() const {
	return (host);
}

std::string Request::getConnection() const {
	return (connection);
}

std::string Request::getSave_buffer() const {
	return (save_buffer);
}

void	Request::setStatus(int status) {
	this->status = status;
}

void	Request::addSave_buffer(const char *buffer) {
	save_buffer += buffer;
}

/* ************************************************************************* */
/* ******************************** Parsing ******************************** */
/* ************************************************************************* */

void	Request::parse_request() {
	method = extract_line(save_buffer, ' ');
	target = extract_line(save_buffer, ' ');
	version = extract_line(save_buffer, '\r');
	host = extract_header(save_buffer);
	agent = extract_header(save_buffer);
	media = extract_header(save_buffer);
	connection = extract_elem("Connection:", "\r", save_buffer);
	std::cout << "connection : " << connection << std ::endl;
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

void	Request::send_response(int socket_fd) {
	
	std::stringstream stream;
	std::string       tmp;
	
	lenght = body.size();
	// std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	stream << lenght;
	stream >> tmp;
	response_content += tmp + "\r\n\r\n" + body;
	write(socket_fd, response_content.c_str(), response_content.size());
	status = SENT;
}

void	Request::build_response(int socket_fd, t_conf &conf, ErrorPages &error) {
	
	std::ifstream     file;

	file.open(target);
	if (file.is_open()) { //GERER ERREUR si pas bon 
		std::getline(file, body, '\0');
		// a verifier text/html si css jss ou utilise avec curl ou http...
		response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	}
	else
		error.fill_error(body, response_content, "404", conf);
	send_response(socket_fd);
}

//  parse request from client and send back response 
int  Request::handle_request(int socket_fd, t_conf &conf, ErrorPages &error)
{
	// /* A EFFACER UNE FOIS PARSING DONE */
	conf.err_pgs["404"] = "pages/error_pages/error_404.html";
	conf.err_pgs["405"] = "pages/error_pages/error_405.html";
	// /* ********************************* */

	std::string index = look_for_location(conf); // si pas trouve index = ""
	// std::cout << "index = " << index << std::endl;
	if (index.empty())
	{
		if (conf.root_dir.empty()) {
			error.fill_error(body, response_content, "404", conf);
			return (send_response(socket_fd), 1);
		}
		else {
			if (conf.root_dir.back() == '/')
				target.replace(0, 1, conf.root_dir);
			else	
				target.insert(0, conf.root_dir);
		}
	}
	/* AVANT D AJOUTER LE PATH => check method ok */
	else {
		if (!check_allow_method(conf, index)) {
			error.fill_error(body, response_content, "405", conf);
			return (send_response(socket_fd), 1);
		}
		add_path(conf, index); // GET POST DELETE
	}
	// std::cout << "target " << target << std::endl;
	return (build_response(socket_fd, conf, error), 1); // GET ONLY ICI
}

/* ************************************************************************* */
/* ******************************* Location ******************************** */
/* ************************************************************************* */

/*
Recherche pour la location de la plus precise a la moins precise dans le serveur
*/
// PENSER A TESTER SI / TOUJOURS AVEC CURL DANS REQUETE COMME POUR HTTP
std::string Request::look_if_location(std::string &target, t_conf & conf) {

	// std::cout << "Look_for_location avec target = " << target << std::endl;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(target);
	if (it != conf.location.end()) {
		// std::cout << "FIND : " << it->first << std::endl;
		return (it->first);	
	}
	std::string s = target;
	if (target.back() == '/')
		s.erase(s.length() - 1, 1); 
	else {
		size_t found = s.rfind('/');
		if (found == std::string::npos) {
			// std::cout << "NOT FOUND" << std::endl; // si not found + pas de root globale pour le serveur => page d erreur en dur 
			return ("");
		}
		s.erase(found + 1, s.length() - found + 1);
	}
	return (look_if_location(s, conf));
}

/*
Cherche si une location = correspondante a la target existe, sinon appelle look_if_location
*/
// A FAIRE : VOIR COMMENT GERER =/loc et =/loc/index.html
std::string Request::look_for_location(t_conf & conf) {
	
	std::string s = "=" + target;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	// std::cout << "hey" << std::endl;
	return (look_if_location(target, conf));
	//on recupere l index de la location :)
}

/* Supprime la location et la remplace par la root */
void	Request::add_path(t_conf & conf, std::string &index) {
	
	std::string root;
	
	if (conf.location[index].find("root") == conf.location[index].end())
		root = conf.root_dir;
	else
		root  = conf.location[index]["root"];
	if(index.back() == '/' && root.back() != '/')
		root += "/";
	if (index.front() == '=')
		index.erase(0, 1);
	target.replace(target.find(index), index.length(), root);
	// std::cout << "target : " << target << std::endl;
}

/* ************************************************************************* */
/* ********************************* Method ******************************** */
/* ************************************************************************* */

bool	Request::check_allow_method(t_conf &conf, std::string &index) {
	std::string	method[3] = {"GET", "POST", "DELETE"};
	int i;
	
	for (i = 0; i < 3; i++) {
		if (this->method == method[i])
			break;
	}
	if (i < 3) {
		if (conf.location[index].find("allow_methods") != conf.location[index].end()) {
			if (conf.location[index]["allow_methods"].find(this->method) == std::string::npos)
				return (false);
		}
		return (true);
	}
	return (false);
}

/* ************************************************************************* */
/* ********************************* Utils ********************************* */
/* ************************************************************************* */

std::string Request::extract_line(std::string & buff, char delim) const
{
	int end = buff.find(delim);
	std::string tmp (buff.substr(0, end));
	if (delim == '\r')
	  end += 1;
	buff.erase(0, end + 1);
	return(tmp);
}

std::string Request::extract_header(std::string & buff) const
{
	int end = buff.find(' ');
	buff.erase(0, end + 1);
	end = buff.find('\r');
	std::string tmp (buff.substr(0, end));
	buff.erase(0, end + 2);
	return (tmp);
}

std::string Request::extract_elem(std::string const & elem, std::string const & delim, std::string & buff) const {
	int begin = buff.find(elem);
	int end = buff.find(delim);
	std::string tmp (buff.substr(begin, end + 1));
	std::cout << "elem line : |" << tmp  << "|" << std::endl;
	return (extract_header(tmp));
}
