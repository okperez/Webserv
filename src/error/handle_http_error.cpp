/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_http_error.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 15:10:48 by garance           #+#    #+#             */
/*   Updated: 2024/06/19 14:33:39 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

void	create_map_error(std::map<std::string, std::string> &map_error) {
	map_error["404"] = " Not Found";
	map_error["405"] = " Method Not Allowed";
}

void	err_not_found(std::string &body, std::string &code, std::map<std::string, std::string> map_error) {
	body = "<!DOCTYPE html>\n<html>\n<head>\n\t<title>";
	body += map_error[code];
	body += "</title>\n</head>\n<body>\n\t<h1>";
	body += code;
	body += "</h1>\n\t<h1>";
	body += map_error[code];
	body += "</h1>\n</body>";
}

void	fill_error(std::string &body, std::string &response, std::string code, t_conf &conf, std::map<std::string, std::string> map_error) {
    
    if (conf.err_pgs.find(code) == conf.err_pgs.end())
		err_not_found(body, code, map_error);
    else {
        std::ifstream file;

        file.open(conf.err_pgs[code]);
        if (file.is_open())
		    std::getline(file, body, '\0');
        else
			err_not_found(body, code, map_error);
    }
    response = "HTTP/1.1 ";
    response += code;
    response += map_error[code];
    response += "\r\nContent-Type: text/html\r\nContent-Length: "; // attention si css jss....
}
