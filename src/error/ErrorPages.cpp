/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPages.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/07/31 12:22:24 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

ErrorPages::ErrorPages() {
    map_error["200"] = " OK";
    map_error["300"] = " Multiple Choices";
    map_error["301"] = " Moved Permanently";
    map_error["302"] = " Found";
    map_error["303"] = " See Other";
    map_error["304"] = " Not Modified";
    map_error["305"] = " Use Proxy";
    map_error["306"] = " Switch Proxy";
    map_error["307"] = " Temporary Redirect";
    map_error["308"] = " Permanent Redirect";
    map_error["400"] = " Bad Request";
    map_error["403"] = " Forbidden";
    map_error["404"] = " Not Found";
	map_error["405"] = " Method Not Allowed";
	map_error["406"] = " Not Acceptable";
	map_error["408"] = " Request Timeout";
	map_error["411"] = " Length Required";
    map_error["413"] = " Request Entity Too Large";
    map_error["415"] = " Unsupported Media Type";
    map_error["500"] = " Internal Server Error";
    map_error["501"] = " Not implemented";
    map_error["503"] = " Service Unavailable";
	map_error["505"] = " HTTP Version not supported";
	map_error["508"] = " Loop Detected";
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
	if (map_error.find(code) != map_error.end())
		return (map_error[code]);
	return ("");
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

void	ErrorPages::err_not_found(Response &response, std::string &code) {
	response.setBody("<!DOCTYPE html>\n<html>\n<head>\n\t<title>");
	response.setBody(map_error[code]);
	response.setBody("</title>\n</head>\n<body>\n\t<h1>");
	response.setBody(code);
	response.setBody("</h1>\n\t<h1>");
	response.setBody(map_error[code]);
	response.setBody("</h1>\n</body>\n</html>");
	response.setContent_type("html"); // ICI OK CAR GENERE AUTOMATIQUEMENT html
}

void	ErrorPages::fill_error(Response &response, std::string code, t_conf &conf) {
    
    if (conf.err_pgs.find(code) == conf.err_pgs.end())
		err_not_found(response, code);
    else {
        std::ifstream file;
		
        file.open(conf.err_pgs[code].data()); // OK NO LEAK + MEMMORY + ERROR SET
        if (file.is_open()) {
			response.setBody(file);
			std::string	type = Request::extract_extension(conf.err_pgs[code]);
			response.setContent_type(type);
		}
        else
			err_not_found(response, code);
    }
	response.setStatus(code, map_error[code]);
	response.setError(true);
}

void	ErrorPages::fill_significant_error(Response &response, std::string code, t_conf &conf) {
	fill_error(response, code, conf);
	response.setConnectiontoclose();
	// response.print();
}

void	ErrorPages::fill_error(Response &response, std::string code) {
    
    err_not_found(response, code);
	response.setStatus(code, map_error[code]);
	response.setError(true);
}

void	ErrorPages::fill_significant_error(Response &response, std::string code) {
    
    fill_error(response, code);
	response.setConnectiontoclose();
	// response.print();
}

void	ErrorPages::fill_redir(Response &response, std::string const &code, std::string const &redir) {
    
	response.setStatus(code, map_error[code]);
	response.setLocation(redir);
}
