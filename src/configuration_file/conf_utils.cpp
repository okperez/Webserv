/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conf_utils.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 11:58:50 by operez            #+#    #+#             */
/*   Updated: 2024/07/09 09:06:08 by galambey         ###   ########.fr       */
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

void    print_all_struct(std::vector<t_conf> & conf, int count)
{
    for (int i = 0; i < count; i++)
    {
        std::cout << "Content server " << i << ":\n\n";
        for (std::vector<std::string>::iterator it = conf[i].ipv4_port.begin(); it != conf[i].ipv4_port.end(); it++)
        {
            std::cout << "ipv4_port | " << *it << std::endl;
        }
		std::cout << std::endl;
        if (!conf[i].host.empty())
            std::cout << "host | " << conf[i].host << std::endl;
        std::cout << std::endl;
        if (!conf[i].server_name.empty())
        std::cout << "Server name | " << conf[i].server_name << std::endl;
        std::cout << std::endl;
        if (!conf[i].root_dir.empty())
        std::cout << "root_dir | " << conf[i].root_dir << std::endl;
        if (!conf[i].ret.empty())
        std::cout << "return | " << conf[i].ret << std::endl;
        std::cout << std::endl;
        for (std::map<std::string, std::map<std::string, std::string>>::iterator it = conf[i].location.begin(); it != conf[i].location.end(); it++)
        {
            std::cout << "Location " << (*it).first << std::endl;
            for (std::map<std::string, std::string>::iterator its = (*it).second.begin(); its != (*it).second.end(); its++)
            {
                std::cout << (*its).first << " | " << (*its).second << std::endl;
            }
            std::cout << std::endl;
        }
        for (std::map<std::string, std::string>::iterator it = conf[i].err_pgs.begin(); it != conf[i].err_pgs.end(); it++)
        {
            std::cout << (*it).first << " | "  << (*it).second << std::endl;
        }
        std::cout << "\n\n";
    }
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
