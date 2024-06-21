/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conf_parsing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:58:30 by operez            #+#    #+#             */
/*   Updated: 2024/06/21 15:15:48 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

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
                    int pos = buff.find_first_of("abcdefghijklmnopqrstuvwxyz{}4#");
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

void    split_conf_file(std::list<std::string> & cnf_file, std::list<std::string> split_file[], int server_nbr)
{
    int server = 0;
    int bracket = 0;
    
    // for (auto it = cnf_file.begin(); it != cnf_file.end(); it++)
    // {
            // std::cout << *it << std::endl;
    // }
    
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        if ((*it).find('{') != (*it).npos)
            bracket += 1;
        if ((*it).find('}') != (*it).npos)
        {
            bracket -= 1;
            if (bracket == 0)
            {
                split_file[server].push_back((*it));
                server += 1;
                if (server == server_nbr)
                    return ;
                continue ;
            }  
        }
        split_file[server].push_back((*it));
    }
    // possiblite d'introduire une regle ici  qui dit que si on est pas a conf end
    //(donc qu'il reste des lignes non attribues a un serveur on considere le ficher conf faux)
    
    
    // for (int i = 0; i < 3; i++)
    // {
        // for (auto it = split_file[i].begin(); it != split_file[i].end(); it++)
        // {
            // std::cout << *it << std::endl;
        // }
        // std::cout << "\n\n||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n\n";
    // }
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
        split_conf_file(cnf_file, split_file, server_nbr);
        conf.resize(server_nbr);

        for (int i = 0; i < server_nbr; i++)
        {
            set_conf_struct(split_file[i], conf[i]);
            check_if_missing(conf[i], split_file[i]);
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