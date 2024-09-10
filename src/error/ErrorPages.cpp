/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPages.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/09/05 17:05:05 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

ErrorPages::ErrorPages() {
    _map_error["200"] = " OK";
    _map_error["204"] = " No Content";
    _map_error["300"] = " Multiple Choices";
    _map_error["301"] = " Moved Permanently";
    _map_error["302"] = " Found";
    _map_error["303"] = " See Other";
    _map_error["304"] = " Not Modified";
    _map_error["305"] = " Use Proxy";
    _map_error["306"] = " Switch Proxy";
    _map_error["307"] = " Temporary Redirect";
    _map_error["308"] = " Permanent Redirect";
    _map_error["400"] = " Bad Request";
    _map_error["403"] = " Forbidden";
    _map_error["404"] = " Not Found";
	_map_error["405"] = " Method Not Allowed";
	_map_error["406"] = " Not Acceptable";
	_map_error["408"] = " Request Timeout";
	_map_error["411"] = " Length Required";
    _map_error["413"] = " Request Entity Too Large";
    _map_error["415"] = " Unsupported Media Type";
    _map_error["500"] = " Internal Server Error";
    _map_error["501"] = " Not implemented";
    _map_error["503"] = " Service Unavailable";
	_map_error["505"] = " HTTP Version not supported";
	_map_error["508"] = " Loop Detected";
}

ErrorPages::ErrorPages(const ErrorPages & orig) { (void) orig; }

ErrorPages::~ErrorPages() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */

/* ************************** Assignment Operator  ************************* */

ErrorPages &ErrorPages::operator=(ErrorPages & rhs) {
	(void) rhs;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

std::string	ErrorPages::get_message(std::string const &code) {
	if (_map_error.find(code) != _map_error.end())
		return (_map_error[code]);
	return ("");
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

void	ErrorPages::err_not_found(Response &response, std::string const &code) {
	response.setBody("<!DOCTYPE html>\n<html>\n<head>\n\t<title>");
	response.setBody(_map_error[code]);
	response.setBody("</title>\n</head>\n<body>\n\t<h1>");
	response.setBody(code);
	response.setBody("</h1>\n\t<h1>");
	response.setBody(_map_error[code]);
	response.setBody("</h1>\n</body>\n</html>");
	response.setContent_type("html"); // ICI OK CAR GENERE AUTOMATIQUEMENT html
}

void	ErrorPages::fill_error(Response &response, std::string const code, t_conf &conf) {
    
    if (conf.err_pgs.find(code) == conf.err_pgs.end())
		err_not_found(response, code);
    else {
        std::ifstream file;
		
        file.open(conf.err_pgs[code].data());
        if (file.is_open()) {
			response.setBody(file);
			std::string	type = Request::extract_extension(conf.err_pgs[code]);
			response.setContent_type(type);
		}
        else
			err_not_found(response, code);
    }
	response.setStatus(code, _map_error[code]);
	// response.setError(true);
}

void	ErrorPages::fill_significant_error(Response &response, std::string const code, t_conf &conf) {
	fill_error(response, code, conf);
	response.setConnectiontoclose();
	// response.print();
}

void	ErrorPages::fill_error(Response &response, std::string const code) {
    
    err_not_found(response, code);
	response.setStatus(code, _map_error[code]);
}

void	ErrorPages::fill_significant_error(Response &response, std::string const code) {
    
    fill_error(response, code);
	response.setConnectiontoclose();
}

void	ErrorPages::fill_redir(Response &response, std::string const &code, std::string const &redir) {
    
	response.setStatus(code, _map_error[code]);
	response.setLocation(redir);
}

void	ErrorPages::del_all() {
		_map_error.clear();
}
