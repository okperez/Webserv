/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conf_parsing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:58:30 by operez            #+#    #+#             */
/*   Updated: 2024/06/05 18:14:27 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

std::string extract_conf(std::string & buff)
{
    unsigned long pos = buff.find(' ');
    buff.erase(0, pos + 1);
    pos = buff.find(';');
    if (pos == buff.npos)
        throw ConfFileException("Error: expected ';' at end of declaration");
    std::string tmp (buff.substr(0, pos));
    return (tmp);
}

int     print_error(char const *str)
{
    std::cerr << "Error: " << str << std::endl; 
    return (-1);
}

void    set_conf_struct(std::list<std::string> & cnf_file, t_conf conf)
{

    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        if ((*it).find("listen "))
        {
            if ((*it).find("[..]"))
                 conf.ipv6_port = extract_conf(*it);
            else
                 conf.ipv4_port = extract_conf(*it);
        }
        else
            throw ConfFileException ("Error: missing port number");       
        if ((*it).find("root "))
            conf.root_dir = extract_conf(*it);
        else
            throw ConfFileException ("Error: missing root directory");       
        if ((*it).find("index "))
            conf.files = extract_conf(*it);
        if ((*it).find("server_name "))
            conf.server_name = extract_conf(*it);
        //checker if root or listen or location is null, missing
    }
}

bool    is_white_space(char c, char d)
{
    if (c == d && isspace(c))
        return (true);
    return (false);
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
                cnf_file.push_back(buff);
        }
    }
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
    conf.location = "";
}

int     parse_conf_file(char *argv)
{
    std::ifstream               file;
    std::string                 buff;
    std::list<std::string>      cnf_file;
    t_conf                      conf;

    set_to_null(conf);
    clear_file(cnf_file, argv);
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        std::cout << *it << std::endl;
    }
    try
    {
        set_conf_struct(cnf_file, conf);
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