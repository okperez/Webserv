/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   allow_method.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 12:02:41 by galambey          #+#    #+#             */
/*   Updated: 2024/06/29 12:10:12 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "../../include/webserv.hpp"

// bool	check_allow_method(t_request &request, t_conf &conf, std::string &index) {
// 	std::string	method[3] = {"GET", "POST", "DELETE"};
// 	int i;
	
// 	for (i = 0; i < 3; i++) {
// 		if (request.method == method[i])
// 			break;
// 	}
// 	if (i < 3) {
// 		if (conf.location[index].find("allow_methods") != conf.location[index].end()) {
// 			if (conf.location[index]["allow_methods"].find(request.method) == std::string::npos)
// 				return (false);
// 		}
// 		return (true);
// 	}
// 	return (false);
// }
