/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_http_error.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 15:10:48 by garance           #+#    #+#             */
/*   Updated: 2024/06/19 11:09:38 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

void	create_map_error(std::map<std::string, std::string> &map_error) {
	map_error["404"] = " Not Found";
}

		// response_content += "200 OK\r\nContent-Type: text/html\r\nContent-Length: "

void	fill_error(std::string &body, std::string &response, std::string code, t_conf &conf, std::map<std::string, std::string> map_error) {
    
    if (conf.err_pgs.find(code) == conf.err_pgs.end())
        std::cout << "1.IMPLEMENTER EN DUR ERROR PAGE" << std::endl; // A FAIRE
    else {
        std::ifstream file;

        file.open(conf.err_pgs[code]);
        if (file.is_open())
		    std::getline(file, body, '\0');
        else
            std::cout << "2.IMPLEMENTER EN DUR ERROR PAGE" << std::endl; // A FAIRE
    }
    response = "HTTP/1.1 ";
    response += code;
    response += map_error[code];
    response += "\r\nContent-Type: text/html\r\nContent-Length: "; // attetion si css jss....
}
