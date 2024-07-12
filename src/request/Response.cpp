/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 14:02:31 by galambey          #+#    #+#             */
/*   Updated: 2024/07/12 09:37:12 by galambey         ###   ########.fr       */
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
	_location = rhs._location;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

void	Response::setStatus(std::string const & code, std::string const & tittle) {
	_status = code + tittle;
}

void		Response::setContent_type(std::string const &type, std::map<std::string, std::vector<std::string> > &media) {
	for (std::map<std::string, std::vector<std::string> >::iterator it=media.begin() ; it != media.end(); it++) {
		for (std::vector<std::string>::iterator jt = it->second.begin(); jt != it->second.end(); jt++)
			if (*jt == type)
				_content_type = it->first + "/" + type;
	}
	// ET SI PAS DE CORRESPONDANCE? ATTENDRE ORLOU POUR MERGE ET RECUPERER MEDIA SANS SERVER
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

void	Response::setBody(std::string const & s) {
	_body += s;	
}

void	Response::reinitBody() {
	_body = "";	
}

void	Response::setBody(std::ifstream &file) {
	std::getline(file, _body, '\0');
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

std::string Response::build_response() const {
	std::string title = "\e[36m";
	std::string reset = "\e[0m";
	std::string response = "HTTP/1.1 ";
	std::string delim = "\r\n";
	
	response += _status + delim;
	std::cout << title<< "********** RESPONSE **********" << std::endl;
	std::cout << response;
	std::cout << "******************************" << reset << std::endl;
	if (!_location.empty())
		response += "Location: " + _location + delim;
	if (!_content_type.empty())
		response += "Content-Type: " + _content_type + delim;
	response += "Content-Length: " + _content_length + delim + delim;
	if (!_body.empty())
		response += _body;
	return (response);
}

void	Response::print() const {
	std::cout << "_start_line : " << _status << std::endl;
	std::cout << "_content_type : " << _content_type << std::endl;
	std::cout << "_content_length : " << _content_length << std::endl;
	std::cout << "_body : " << _body << std::endl;
}
