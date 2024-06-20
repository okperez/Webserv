/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conf_parsing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:58:30 by operez            #+#    #+#             */
/*   Updated: 2024/06/20 14:49:57 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

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

void    check_for_root(std::map<std::string, std::map<std::string, std::string>> & location)
{
    int flag;
    for (std::map<std::string, std::map<std::string, std::string>>::iterator it = location.begin(); it != location.end(); it++)
    {
        flag = 0;
        for (std::map<std::string, std::string>::iterator its = (*it).second.begin(); its != (*it).second.end(); its++)
        {
            if ((*its).first == "root" && !(*its).second.empty())
                flag = 1;
            else if ((*its).first == "return" && !(*its).second.empty())
                flag = 1;
        }
        if (flag == 0)
            throw ConfFileException("Error: missing root directory");
    }
}

void    check_if_missing(t_conf & conf, std::list<std::string> & cnf_file)
{
    int location_methods = 0;
    int total_methods = 0;
    
    if (conf.ipv4_port.size() == 0)
        throw ConfFileException ("Error: missing port number");
    if (conf.max_body_size.empty())
        throw ConfFileException ("Error: missing body size number");
    if (conf.root_dir.empty())
        check_for_root(conf.location);
    if (conf.files == "")
        conf.files = "index.html";
    if (conf.server_name.empty())
        conf.server_name = get_ip_address();
    for (std::map<std::string, std::map<std::string, std::string>>::iterator it = conf.location.begin(); it != conf.location.end(); it++)
    {
        for(std::map<std::string, std::string>::iterator its = (*it).second.begin(); its != (*it).second.end(); its++)
        {
            if (is_allow_methods((*its).first))
                location_methods++;
        }
    }
    total_methods = std::count_if(cnf_file.begin(), cnf_file.end(), is_allow_methods);
    if (total_methods > location_methods)
        throw ConfFileException ("Error: allow_methods present outside location");
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
        (*it).erase(std::unique((*it).begin(), (*it).end(), is_white_space), (*it).end());
        if ((*it).find('#') == 0)
        {
            it = cnf_file.erase(it);
            continue ;
        }
        it++;
    }
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        if (((*it) == "server" || (*it) == "server ") && *std::next(it) == "{")
        {
            *it = "server{";
            it = cnf_file.erase(std::next(it));
        }
    }
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

void    split_conf_file(std::list<std::string> & cnf_file, std::list<std::string> split_file[])
{
    int server = 0;

    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        if (((*it).find("server{") != (*it).npos|| (*it).find("server {") != (*it).npos) && it != cnf_file.begin())
            server += 1;  
        split_file[server].push_back((*it));
    }
}

void    compare_server(std::vector<t_conf> & conf, int  & server_nbr)
{
    std::vector<t_conf>::iterator it = conf.begin();
    std::vector<t_conf>::iterator its;
    
    while (it != conf.end())
    {
        its = it + 1;
        while (its != conf.end())
        {
            if (is_equal((*it), (*its)))
            {
                its = conf.erase(its);
                server_nbr--;
            }
            else
                its++;
        }
        it++;
    }
}

int     handle_conf_file(char *argv, std::vector<t_conf> & conf)
{
    std::ifstream               file;
    std::string                 buff;
    std::list<std::string>      cnf_file;
    int                         server_nbr;

    try
    {
        clear_file(cnf_file, argv);
		check_syntax(cnf_file);
        server_nbr = count_server(cnf_file);
        std::list<std::string>      split_file[server_nbr];
        split_conf_file(cnf_file, split_file);
        conf.resize(server_nbr);

        for (int i = 0; i < server_nbr; i++)
        {
            set_conf_struct(split_file[i], conf[i]);
            check_if_missing(conf[i], split_file[i]);
            // std::cout << "\n\n|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n\n";
        }
        compare_server(conf, server_nbr);
    }
    catch(const std::exception & e)
    {
        std::cerr << e.what() << '\n';
        return (-1);
    }
    print_all_struct(conf, server_nbr);
    return (0);
}

// gerer cas ou allow method dns server
// comparer nombre total de allow method dans fichier vs location (tout server)
