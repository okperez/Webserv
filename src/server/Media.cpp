/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Media.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/08/29 11:49:02 by operez           ###   ########.fr       */
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
    types["application"].push_back("octet-stream"); // DIT AU NAVIGATEUR DE NE PAS ESSAYER DE LE TRAITER MAIS DE LE TELECHARGER
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

std::map<std::string, std::vector<std::string> >	& Media::getTypes()
{
	return (types);
}


std::string	Media::match_type(std::string const &type) {
	
	for (std::map<std::string, std::vector<std::string> >::iterator it = types.begin(); it != types.end(); it++) {
		for (std::vector<std::string>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
			if (*jt == type)
				return (it->first + "/" + *jt);
		}
	}
	return ("");
}
