/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 14:02:31 by galambey          #+#    #+#             */
/*   Updated: 2024/09/02 17:29:30 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Response::Response() {
	_error = false;
}

Response::Response(const Response & orig) { (void) orig; }

Response::~Response() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */

/* ************************** Assignment Operator  ************************* */

Response &Response::operator=(Response const & rhs) {
	_status = rhs._status;
	_content_type = rhs._content_type;
	_content_length = rhs._content_length;
	_body = rhs._body;
	_location = rhs._location;
	_connection = rhs._connection;
	_cookie = rhs._cookie;
	_error = rhs._error;
	auth_media = rhs.auth_media;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

void	Response::setAuthmedia(Media *auth_media) {
	this->auth_media = auth_media;
}

void	Response::setStatus(std::string const & code, std::string const & tittle) {
	_status = code + tittle;
}

void	Response::setStatus(std::string const & code, ErrorPages &error) {
	_status = code + error.get_message(code);
}

bool	Response::setContent_type(std::string const &type) {
	
	_content_type = auth_media->match_type(type);
	std::cout << "REVOIR CONTENT_TYPE POUR RECUP IMG JPG< XPM, ... ajouter dans media?\n"; 
	if (_content_type.empty())
		return (false);
	return (true);
}

void	Response::setContent_length() {
	std::stringstream stream;

	int length = _body.size();
	stream << length;
	stream >> _content_length;
}

void	Response::setLocation(std::string const & s) {
	_location = s;
}

void	Response::setConnectiontoclose() {
	_connection = "close";
}

void	Response::setBody(std::string const & s) {
	_body += s;	
}

void	Response::reinitBody() {
	_body = "";	
}

void	Response::setBody(std::ifstream &file) {
	std::getline(file, _body, '\0');
}

void	Response::setCookie(std::string str) {
	_cookie.push_back(str);
}

void	Response::setError(bool err) {
	_error = err;
}

bool	Response::getError() {
	return (_error);
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

std::string Response::build_response()
{
	std::string title = "\e[36m";
	std::string reset = "\e[0m";
	std::string response = "HTTP/1.1 ";
	std::string delim = "\r\n";
	response += _status + delim;
	std::cout << title<< "********** RESPONSE **********" << std::endl;
	std::cout << response;
	std::cout << "******************************" << reset << std::endl;

	for (std::vector<std::string>::iterator it = _cookie.begin(); it != _cookie.end(); it++)
		response += (*it) + delim;
	if (!_location.empty())
		response += "Location: " + _location + delim;
	if (!_content_type.empty())
		response += "Content-Type: " + _content_type + delim;
	if (!_connection.empty())
		response += "Connection: " + _connection + delim;
	response += "Content-Length: " + _content_length + delim + delim;
	if (!_body.empty())
		response += _body;
	response += "\r\n";
	return (response);
}

void	Response::print() {
	std::cout << "_start_line : " << _status << std::endl;
	std::cout << "_content_type : " << _content_type << std::endl;
	std::cout << "_content_length : " << _content_length << std::endl;
	std::cout << "_connection : " << _connection << std::endl;
	std::cout << "_body : " << _body << std::endl;
	// for (std::vector<std::string>::iterator it = _cookie.begin(); it != _cookie.end(); it++)
	// 	std::cout << "_cookie: " << (*it) << std::endl;
	// std::cout << "_cookie :" << _cookie << std::endl;
}
