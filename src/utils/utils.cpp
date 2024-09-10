/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/02 11:01:35 by garance           #+#    #+#             */
/*   Updated: 2024/09/06 15:22:22 by garance          ###   ########.fr       */
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
		if (found2 != 0 && found2 != std::string::npos) {
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

void	deleteArgs(char *const argv[], char *const envp[])
{
		for (int i = 0; i < 7; i++)
			delete [] envp[i];
		delete [] envp;
		delete [] argv;
}

void	deleteArr(char *const env[], int index)
{
	for (int i = 0; i < index; i++)
		delete env[i];
}

void	setExtensions(std::map<std::string, char const *> & extensions)
{
	extensions["py"] = "/usr/bin/python3";
	extensions["php"] = "/usr/bin/php";
	extensions["rb"] = "/usr/bin/rb";
	extensions["js"] = "/usr/bin/node";
	extensions["pl"] = "/usr/bin/perl";
	extensions["java"] = "/usr/bin/java";
	extensions["swift"] = "/usr/bin/swift";
	extensions["lua"] = "/usr/bin/lua";
	extensions["r"] = "/usr/bin/R";
	extensions["json"] = "/usr/bin/jq";
	extensions["go"] = "/usr/bin/go";
	extensions["rs"] = "/usr/bin/rustc";
}
