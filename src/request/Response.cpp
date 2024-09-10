/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 14:02:31 by galambey          #+#    #+#             */
/*   Updated: 2024/09/05 16:09:44 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor &Destructor *********************** */
/* ************************************************************************* */

Response::Response() {}

Response::Response(const Response &orig) { (void) orig; }

Response::~Response() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */

/* ************************** Assignment Operator  ************************* */

Response &Response::operator=(Response const &rhs) {
	_status = rhs._status;
	_content_type = rhs._content_type;
	_content_length = rhs._content_length;
	_body = rhs._body;
	_location = rhs._location;
	_connection = rhs._connection;
	_cookie = rhs._cookie;
	_auth_media = rhs._auth_media;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

void	Response::setAuthmedia(Media *auth_media) {
	this->_auth_media = auth_media;
}

void	Response::setStatus(std::string const &code, std::string const &tittle) {
	_status = code + tittle;
}

void	Response::setStatus(std::string const &code, ErrorPages &error) {
	_status = code + error.get_message(code);
}

void	Response::setContent_type(std::string const &type) {
	
	if (!type.empty())
		_content_type = _auth_media->match_type(type);
	if (_content_type.empty())
		_content_type = "application/octet-stream";
}

void	Response::setContent_length() {
	std::stringstream stream;

	int length = _body.size();
	stream << length;
	stream >> _content_length;
}

void	Response::setLocation(std::string const &s) {
	_location = s;
}

void	Response::setConnectiontoclose() {
	_connection = "close";
}

size_t	Response::setBody(std::string const &s) {
	_body += s;
	return (s.length()); // VERIF SI N AJOUTE PAS DE \0
}

void	Response::reinitBody() {
	_body = "";	
}

size_t	Response::setBody(std::ifstream &file) {
	
	std::stringstream ss;

	ss << file.rdbuf();
	_body = ss.str();
	return (_body.length());
}

void	Response::setCookie(std::string const &str) {
	_cookie.push_back(str);
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

std::string Response::build_response(size_t &body_len)
{
	
	std::string response;
	std::string delim = "\r\n";
	size_t	tmp;

	response.append("HTTP/1.1 "+ _status + delim);
	print_first_line(response);
	for (std::vector<std::string>::iterator it = _cookie.begin(); it != _cookie.end(); it++)
		response.append(*it + delim);
	if (!_location.empty())
		response.append("Location: " + _location + delim);
	if (!_content_type.empty())
		response.append("Content-Type: " + _content_type + delim);
	if (!_connection.empty())
		response.append("Connection: " + _connection + delim);
	response.append("Content-Length: " + _content_length + delim + delim);
	tmp = response.size();
	if (!_body.empty())
		response.append(_body);
	response.append(delim);
	body_len += tmp + 2;
	return (response);
}

void	Response::print_first_line(std::string const &response) const {
	
	std::string title = "\e[36m";
	std::string reset = "\e[0m";
	
	std::cout << title<< "********** RESPONSE **********" << std::endl;
	std::cout << response;
	std::cout << "******************************" << reset << std::endl;
}
