/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPages.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/06/29 11:32:45 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

ErrorPages::ErrorPages() {
    map_error["404"] = " Not Found";
	map_error["405"] = " Method Not Allowed";
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


/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

void	ErrorPages::err_not_found(std::string &body, std::string &code) {
	body = "<!DOCTYPE html>\n<html>\n<head>\n\t<title>";
	body += map_error[code];
	body += "</title>\n</head>\n<body>\n\t<h1>";
	body += code;
	body += "</h1>\n\t<h1>";
	body += map_error[code];
	body += "</h1>\n</body>";
}

void	ErrorPages::fill_error(std::string &body, std::string &response, std::string code, t_conf &conf) {
    
    if (conf.err_pgs.find(code) == conf.err_pgs.end())
		err_not_found(body, code);
    else {
        std::ifstream file;

        file.open(conf.err_pgs[code]);
        if (file.is_open())
		    std::getline(file, body, '\0');
        else
			err_not_found(body, code);
    }
    response = "HTTP/1.1 ";
    response += code;
    response += map_error[code];
    response += "\r\nContent-Type: text/html\r\nContent-Length: "; // attention si css jss....
}
