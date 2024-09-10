/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conf_utils.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:58:50 by operez            #+#    #+#             */
/*   Updated: 2024/09/10 11:34:30 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

bool    is_equal(t_conf & conf1, t_conf & conf2)
{
    int flag = 0;
    if (conf1.server_name == conf2.server_name)
        flag++;
    if ((conf1.ipv4_port.size() == conf2.ipv4_port.size()))
    {
        if (std::equal(conf1.ipv4_port.begin(), conf1.ipv4_port.end(), conf2.ipv4_port.begin()))
            flag++;
    }
    if (conf1.host == conf2.host)
        flag++;
    if (flag == 3)
        return true;
    return false ;
}

void    fill_map(std::map<int, std::string> & map, std::string str)
{
    std::string key = str.substr(0, str.find(' '));
    str.erase(0, str.find(' ') + 1);
    std::string value = str.substr(0, str.npos);
    map[ft_stoi(key)] = value; 
}

bool    is_allow_methods(std::string str)
{
    if (str.find("allow_methods") != str.npos)
        return true;
    return false;
}

int     print_error(char const *str)
{
    std::cerr << "Error: " << str << std::endl; 
    return (-1);
}

bool    is_white_space(char c, char d)
{
    if (c == d && isspace(c))
        return (true);
    return (false);
}

void    clear_space(std::string & type)
{
	type.erase(remove_if(type.begin(), type.end(), isspace), type.end());
}

int ft_stoi( std::string const & s ) {
	long int 	n;
	int 		digit = 0;
	int 		i;
	
	for (i = 1 * (s[0] == '-'); s[i] == '0'; i++) {}
	while (s[i]) {
		if (!isdigit(s[i]))
			throw (NotAnIntException());
		digit++;
		i++;
	}
	std::istringstream(s) >> n;
	if (n < std::numeric_limits<int>::min() || n > std::numeric_limits<int>::max() || digit > 10)
		throw (NotAnIntException());
	return (static_cast<int>(n));
}
