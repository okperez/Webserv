/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 14:02:31 by galambey          #+#    #+#             */
/*   Updated: 2024/07/01 16:53:30 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Response::Response() {}

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
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

void	Response::setStatus(std::string const & code, std::string const & tittle) {
	_status = code + tittle;
}
void	Response::setContent_type(std::string const & s) {
	_content_type = s;
}
void	Response::setContent_length() {
	std::stringstream stream;

	int length = _body.size();
	stream << length;
	stream >> _content_length;
}
void	Response::setBody(std::string const & s) {
	_body += s;	
}

void	Response::setBody(std::ifstream &file) {
	std::getline(file, _body, '\0');
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

std::string Response::build_response() const {
	std::string response = "HTTP/1.1 ";
	std::string delim = "\r\n";
	
	response += _status + delim;
	response += "Content-Type: " + _content_type + delim;
	response += "Content-Length: " + _content_length + delim + delim;
	response += _body;
	// std::cout << "response = |" << response << "|" << std::endl;
	return (response);
}

void	Response::print() const {
	std::cout << "_start_line : " << _status << std::endl;
	std::cout << "_content_type : " << _content_type << std::endl;
	std::cout << "_content_length : " << _content_length << std::endl;
	std::cout << "_body : " << _body << std::endl;
}
