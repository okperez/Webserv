/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:01:35 by garance           #+#    #+#             */
/*   Updated: 2024/07/11 14:12:45 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void	strtovect(std::string s, std::vector<std::string> & v, std::string const & delim) {
    size_t	found;

	if (s.empty())
		return ;
	while (1) {
		found = s.find(delim);
		if (found == 0) {
			if (s.length() == delim.length())
				return ;
			s.erase(0, delim.length());
			continue;
		}
		v.push_back(s.substr(0, found));
		s.erase(0, found);
		if (found == std::string::npos)
			return ;
	}
}

void	strtomap(std::string s, std::map<std::string, std::vector<std::string> > & map, std::string const & delimstr, std::string const & delimmap) {
    size_t		found, found2;
    std::string	tmp, index, content;

	if (s.empty())
		return ;
	while (1) {
		found = s.find(delimstr);
		if (found == 0) {
			if (s.length() == delimstr.length())
				return ;
			s.erase(0, delimstr.length());
			continue;
		}
		tmp = s.substr(0, found);
		s.erase(0, found);
		
		
		found2 = tmp.find(delimmap);
		if (found2 == 0)
			std::cout << "A IMPLEMENTER" << std::endl;
		else if (found2 == std::string::npos)
			std::cout << "A IMPLEMENTER" << std::endl;
		else {
			index = tmp.substr(0, found2);
			content = tmp.substr(found2 + delimmap.length());
			map[index].push_back(content);
		}
		if (found == std::string::npos)
			return ;
	}
}

char	strback(std::string const & s) {
	return (s[s.length() - 1]);
}
