/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   adapt_host.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/03 12:08:19 by operez            #+#    #+#             */
/*   Updated: 2024/09/03 12:44:55 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

std::string	int_to_str(int n) {
	std::string s;
	std::ostringstream oss(s);
	
	oss << n;
	return (oss.str());
}

/* Checke si adresse ip de l host a un format valid + supprime les 0 inutiles. ex : 127.001.001.1 => 127.1.1.1*/
void	adapt_host(std::string &s) {
	
	int n;
	unsigned char c;
	
	if (s == "localhost")
		return ;
	std::istringstream iss(s);
	s = "";
	for (int j = 0; j < 4; j++)
	{
		n = -1;
		iss >> n;
		if (n < 0 || n > 255)
			throw (ConfFileException("Not a valid host"));
		s += int_to_str(n);
		if (j < 3)
		{
			c = -1;
			if (iss.eof())
				throw (ConfFileException("Not a valid host"));
			iss >> c;
			if (c != '.')
				throw (ConfFileException("Not a valid host"));
			s += c;
		}
		else if (j == 3 && !iss.eof())
			throw (ConfFileException("Not a valid host"));
	}
}

void	str_tolower(std::string & s) {
	for (int i = 0; s[i]; i++)
		s[i] = tolower(s[i]); 
}

void        handle_host(std::vector<t_conf> & conf)
{
    for(std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++)
    {
		if (!it->host.empty())
			str_tolower(it->host); 
		if (!it->server_name.empty())
			str_tolower(it->server_name);
		strtovect(it->files, it->files_vect, " ");
		adapt_host((*it).host);
    }
}