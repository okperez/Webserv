/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Media.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/09/10 11:29:25 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Media::Media() {
    _types["text"].push_back("html");
    _types["text"].push_back("css");
    _types["text"].push_back("plain");
    _types["text"].push_back("*");
    _types["application"].push_back("octet-stream"); // DIT AU NAVIGATEUR DE NE PAS ESSAYER DE LE TRAITER MAIS DE LE TELECHARGER
    _types["image"].push_back("jpeg"); // DIT AU NAVIGATEUR DE NE PAS ESSAYER DE LE TRAITER MAIS DE LE TELECHARGER
    _types["image"].push_back("jpg"); // DIT AU NAVIGATEUR DE NE PAS ESSAYER DE LE TRAITER MAIS DE LE TELECHARGER
    _types["image"].push_back("png"); // DIT AU NAVIGATEUR DE NE PAS ESSAYER DE LE TRAITER MAIS DE LE TELECHARGER
    _types["image"].push_back("xpm"); // DIT AU NAVIGATEUR DE NE PAS ESSAYER DE LE TRAITER MAIS DE LE TELECHARGER
    _types["*"].push_back("*");
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
	
	for (std::map<std::string, std::vector<std::string> >::iterator it = _types.begin() ; it != _types.end(); it++) {
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
	return (_types);
}


std::string	Media::match_type(std::string const &type) {
	
	for (std::map<std::string, std::vector<std::string> >::iterator it = _types.begin(); it != _types.end(); it++) {
		for (std::vector<std::string>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
			if (*jt == type)
				return (it->first + "/" + *jt);
		}
	}
	return ("");
}
