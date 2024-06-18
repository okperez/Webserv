/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conf_parsing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:58:30 by operez            #+#    #+#             */
/*   Updated: 2024/06/18 12:39:09 by operez           ###   ########.fr       */
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
    // if (conf.ipv4_port.size() == 0)
        // throw ConfFileException ("Error: missing port number");
    // if (conf.root_dir == "")
        // throw ConfFileException ("Error: missing root directory");
    if (conf.files == "")
    {
        std::cout << "No default file has been set-up, default file will be set to index.html" << std::endl;
        conf.files = "index.html";
    }
    if (conf.server_name.empty())
    {
        conf.server_name = get_ip_address();
        // std::cout << "No server_name has been set-up, server_name will be set to IP adress: ";
        // std::cout << conf.server_name << std::endl;
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
    conf.server_name = "";
    conf.root_dir = "";
    conf.files = "";
}

int     count_server(std::list<std::string> & cnf_file)
{
    std::string str = clear_str(cnf_file);
    int count = 0;
    
    std::string::size_type pos = 0;
    std::string target = "server{";
    while ((pos = str.find(target, pos )) != std::string::npos)
    {
           ++ count;
           pos += target.length();
    }
    return (count);
}   

void    print_all_struct(std::vector<t_conf> & conf, int count)
{
    for (int i = 0; i < count; i++)
    {
        std::cout << "Content server:\n\n";
        for (std::vector<std::string>::iterator it = conf[i].ipv4_port.begin(); it != conf[i].ipv4_port.end(); it++)
        {
            std::cout << "ipv4_port = " << *it << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Server name = " << conf[i].server_name << std::endl;
        std::cout << std::endl;
        std::cout << "root_dir = " << conf[i].root_dir << std::endl;
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
    }
}


void    remove_server_part(std::list<std::string> & cnf_file)
{
    std::list<std::string>::iterator begin = cnf_file.begin();
    std::list<std::string>::iterator end;
    for (std::list<std::string>::iterator it = ++begin; it != cnf_file.end(); it++)
    {
        if ((*it).find("server{") != (*it).npos || (*it).find("server {") != (*it).npos|| (*it).find("server\0") != (*it).npos)
        {
            end = it;
            break ;
        }
    }
    cnf_file.erase(begin, end);
}

int     parse_conf_file(char *argv, std::vector<t_conf> & conf)
{
    std::ifstream               file;
    std::string                 buff;
    std::list<std::string>      cnf_file;
    int                         server_nbr;

    try
    {
        clear_file(cnf_file, argv);
        // for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
        // {
            // std::cout << *it << std::endl;
        // }
		check_syntax(cnf_file);
        server_nbr = count_server(cnf_file);
        // std::cout << "Nbr server = " << server_nbr << std::endl;
        conf.resize(server_nbr);
        for (int i = 0; i < server_nbr; i++)
        {
            set_conf_struct(cnf_file, conf[i]);
            check_if_missing(conf[i]);
            // verifier dans check if missing si lorsque root est pas mentionne qu'il l'est dans chaque block location
            remove_server_part(cnf_file);
        }
    }
    catch(const std::exception & e)
    {
        std::cerr << e.what() << '\n';
        return (-1);
    }
    // print_all_struct(conf, server_nbr);

    // if (buff.empty())
        // return (print_error("Configuration file empty"));
    return (0);
}