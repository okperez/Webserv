/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_syntax.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:58:30 by operez            #+#    #+#             */
/*   Updated: 2024/06/21 14:11:54 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

bool    is_white_space(char c)
{
    if (isspace(c))
        return (true);
    return (false);
}

void	check_bracket(std::list<std::string> & cnf_file)
{
	int			sign = 0;
	int			b = 0;
	
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
		if ((*it).find('{') != (*it).npos)
		{
			sign += 1;
			b += 1;
		}
		else if ((*it).find('}') != (*it).npos)
		{
			sign -= 1;
			b += 1;
		}
		if (sign == -1)
        	throw ConfFileException ("Error: missing bracket");
	}
	if (sign == 1)
    throw ConfFileException ("Error: too many bracket");	
}

void	erase_content(std::string & str, char c)
{
	const size_t pos = str.find(c) + 1;
	str.erase(0, pos);
}

void	check_outside_bracket(std::string str)
{
	while (1)
	{
		erase_content(str, '}');
		if (str.find('{') == str.npos)
			break ;
		std::string extract = str.substr(0, str.find('{'));
		erase_content(str, '{');
		// std::cout << "Extracted sequence =\n" << extract << std::endl;
		if (extract == "" || extract.find("server") != extract.npos
			|| extract.find("location/") != extract.npos || extract.find("error_page") != extract.npos
			|| extract.find("location=") != extract.npos)
			;
		else
			throw ConfFileException("unknown directive");
	}
}

void	check_server_content(std::string str)
{
	static const size_t	pos = str.find("server{");
	str.erase(0, pos);
	static const size_t	r_bracket = str.find('}');

	if (r_bracket > str.find("location{"))
		throw ConfFileException("Error: server context not declared in main");
	std::string sub = str.substr(7, r_bracket - 7);
}

void	check_http_content(std::string str)
{
	static const size_t	pos = str.find("http{");
	str.erase(0, pos);
	static const size_t	r_bracket = str.find('}');

	if (r_bracket > str.find("server{"))
		throw ConfFileException("Error: server context not declared in main");
	std::string sub = str.substr(5, r_bracket - 5);
	if (sub.find("server{") == sub.npos)
		throw ConfFileException("Error: server context not declared in main");
}

std::string	clear_str(std::list<std::string> cnf_file)
{
	std::string	str = "";
	std::string::iterator it;
	
	for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
		str += *it;
	clear_space(str);
	str = str.substr(0, str.find('\0'));
	return (str);
}

void    quick_parsing(std::list<std::string> & cnf_file)
{
    auto it = cnf_file.begin();
    if (*it != "server {" && *it != "server{")	
        throw ConfFileException ("Error: missing server directive");
}

void	check_syntax(std::list<std::string> & cnf_file)
{
	std::string	str;

	check_bracket(cnf_file);
	str = clear_str(cnf_file);
	check_outside_bracket(str);
	quick_parsing(cnf_file);
}