/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:01:35 by garance           #+#    #+#             */
/*   Updated: 2024/07/02 11:33:11 by garance          ###   ########.fr       */
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
			if (s.length() == 1)
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
