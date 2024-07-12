/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Media.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/07/12 17:58:43 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Media::Media() {
    types["text"].push_back("html");
    types["text"].push_back("css");
    types["text"].push_back("plain");
    types["text"].push_back("*");
    types["*"].push_back("*");
}

Media::Media(const Media & orig) { (void) orig; }

Media::~Media() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */

/* ************************** Assignment Operator  ************************* */

Media &Media::operator=(Media & rhs) {
	(void) rhs;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */


/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

bool	Media::is_allow(std::string const &index, std::string const &content) {
	
	for (std::map<std::string, std::vector<std::string> >::iterator it = types.begin() ; it != types.end(); it++) {
		if (it->first == index) {
			for (std::vector<std::string>::iterator jt = it->second.begin(); jt != it->second.end(); jt++)
				if (*jt == content)
					return (true);
		}
	}
	return (false);
}

std::string	Media::match_type(std::string const &type) {
	
	std::cout << "TEST00" << std::endl;
	for (std::map<std::string, std::vector<std::string> >::iterator it = types.begin(); it != types.end(); it++) {
		std::cout << "TEST01" << std::endl;
		for (std::vector<std::string>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
			std::cout << "TEST02" << std::endl;
			if (*jt == type)
				return (std::cout << "TEST03" << std::endl, it->first + "/" + *jt);
		}
	}
	std::cout << "TEST04" << std::endl;
	return ("");
}
