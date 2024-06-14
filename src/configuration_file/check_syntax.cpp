/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_syntax.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:58:30 by operez            #+#    #+#             */
/*   Updated: 2024/06/14 17:02:31 by operez           ###   ########.fr       */
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
		for (std::string::iterator its = (*it).begin(); its != (*it).end(); its++)
		{
			if (*its == '{')
			{
				sign += 1;
				b += 1;
			}
			else if (*its == '}')
			{
				sign -= 1;
				b += 1;
			}
			if (sign == -1)
        		throw ConfFileException ("Error: missing bracket");
		}
    }
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
		if (extract == "" || extract == "events" || extract == "http" || extract == "server"
			|| extract.find("location/") != extract.npos || extract.find("error_page") != extract.npos)
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

void	check_events_content(std::string str)
{
	static const size_t	pos = str.find("events{");
	str.erase(0, pos);
	static const size_t	r_bracket = str.find('}');

	if (r_bracket > str.find("http{"))
		throw ConfFileException("Error: http context not declared in main");
	std::string sub = str.substr(7, r_bracket - 7);
	if (sub.find("worker_connections") == sub.npos)
		std::cout << "Worker_connections directive missing, number of simultaneous connections will be set to default\n";
}

void	check_order(std::string str)
{
	static const size_t	pos_events = str.find("events{");
	static const size_t	pos_http = str.find("http{");
	static const size_t	pos_server = str.find("server{");

	if (pos_events == str.npos|| pos_http == str.npos || pos_server == str.npos)
        throw ConfFileException ("Error: missing context");
	if (!(pos_events < pos_http && pos_http < pos_server))
        throw ConfFileException ("Error: contexts declared in wrong order");
	check_events_content(str);
	
	
}

std::string	clear_str(std::list<std::string> cnf_file)
{
	std::string	str = "";
	std::string::iterator it;
	
	for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
	{
		str += *it;
	}
	// std::cout << "Here is the string:\n" << str << "\n\n";
	it = std::remove_if(str.begin(), str.end(), is_white_space);
	*it = '\0';
	str = str.substr(0, str.find('\0'));
	return (str);
}

void	check_bracket_content(std::list<std::string> cnf_file, std::string & str)
{

}

void	check_syntax(std::list<std::string> & cnf_file)
{
	std::string	str;

	check_bracket(cnf_file);
	str = clear_str(cnf_file);
	check_bracket_content(cnf_file, str);
	// check_order(str);
	check_outside_bracket(str);
}