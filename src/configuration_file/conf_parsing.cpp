/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conf_parsing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:58:30 by operez            #+#    #+#             */
/*   Updated: 2024/06/14 15:31:11 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

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

char*        get_ip_address(void)
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        std::cerr << "Error getting hostname" << std::endl;
    }
    struct hostent *host_entry = gethostbyname(hostname);
    if (host_entry == NULL) {
        std::cerr << "Error getting host entry" << std::endl;
    }
    char *ip = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    return (ip);
}



void    check_if_missing(t_conf & conf)
{
    if (conf.ipv4_port == "" && conf.ipv6_port == "")
        throw ConfFileException ("Error: missing port number");
    if (conf.root_dir == "")
        throw ConfFileException ("Error: missing root directory");
    if (conf.files == "")
    {
        std::cout << "No default file has been set-up, default file will be set to index.html" << std::endl;
        conf.files = "index.html";
    }
    if (conf.server_name.empty())
    {
        conf.server_name = get_ip_address();
        std::cout << "No server_name has been set-up, server_name will be set to IP adress: ";
        std::cout << conf.server_name << std::endl;
    }
}

void    clear_file(std::list<std::string> & cnf_file, char *argv)
{
    std::ifstream               file;
    std::string                 buff;
    file.open(argv);
    if (file.is_open())
    {
        while (file)
        {
            std::getline(file, buff, '\n');
            {
                    int pos = buff.find_first_of("abcdefghijklmnopqrstuvwxyz{}#");
                    buff.erase(0, pos);
            }
            if (!buff.empty())
            {
                cnf_file.push_back(buff);
                buff = "";
            }
        }
    }
    else
        throw ConfFileException("Error: no such configuration file");
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end();)
    {
        std::unique((*it).begin(), (*it).end(), is_white_space);
        if ((*it).find('#') == 0)
        {
            it = cnf_file.erase(it);
            continue ;
        }
        it++;
    }
}

void    set_to_null(t_conf & conf)
{
    conf.ipv4_port = "";
    conf.ipv6_port = "";
    conf.server_name = "";
    conf.root_dir = "";
    conf.files = "";
}

int     parse_conf_file(char *argv)
{
    std::ifstream               file;
    std::string                 buff;
    std::list<std::string>      cnf_file;
    t_conf                      conf;

    try
    {
        set_to_null(conf);
        clear_file(cnf_file, argv);
        // for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
        // {
            // std::cout << *it << std::endl;
        // }
		check_syntax(cnf_file);
        set_conf_struct(cnf_file, conf);
        check_if_missing(conf);
    }
    catch(const std::exception & e)
    {
        std::cerr << e.what() << '\n';
        return (-1);
    }
    // if (buff.empty())
        // return (print_error("Configuration file empty"));
    return (0);
}