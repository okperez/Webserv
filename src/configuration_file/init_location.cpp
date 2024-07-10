/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_location.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 11:14:34 by operez            #+#    #+#             */
/*   Updated: 2024/07/10 10:49:00 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

std::string extract_type(std::string buff)
{
    unsigned long pos = buff.find(' ');
    buff.erase(0, pos + 1);
    pos = buff.find('{');
    std::string tmp (buff.substr(0, pos));
    return (tmp);
}

std::string extract_index(std::string buff)
{
    std::string name = buff.substr(0, buff.find(' '));
    return (name);    
}

void    location_to_conf(std::list<std::string> & location, t_conf & conf)
{
    std::pair<std::string, std::string> pair;
    std::map<std::string, std::string>  path;
    std::string                         type;

    type = extract_type(*(location.begin()));
    clear_space(type);
    for (std::list<std::string>::iterator it = ++location.begin(); it != location.end(); it++)
    {
        if (*it == "{" || *it == "}")
            continue ;
        pair.first = extract_index(*it);
        pair.second = extract_conf(*it, ';');
        path[pair.first] = pair.second;
        
    }
    conf.location.insert(make_pair(type, path));
}

std::list<std::string>  extract_bloc_location(std::list<std::string> & cnf_file, std::list<std::string>::iterator begin, t_conf & conf)
{
    std::list<std::string>::iterator    end;
    std::list<std::string>::iterator    it = begin;
    std::list<std::string>              location;

    while (1)
    {
        if ((*it).find('}') != (*it).npos)
        {
            end = it;
            break ;
        }
        if ((*it).find("return") != (*it).npos)
        {
            if (conf.flag.ret_loc == 1)
            {
                it = cnf_file.erase(it);
                continue ;
            }
            else
                conf.flag.ret_loc = 1;
        }
        it++;
    }
    location.insert(location.end(), begin, end);
    conf.flag.ret_loc = 0;
    return (location);
}

void    handle_locations(std::list<std::string> & cnf_file, t_conf & conf)
{
    std::list<std::string> location;
    
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        if ((*it).find("location") != (*it).npos)
        {
            location = extract_bloc_location(cnf_file, it, conf);
            location_to_conf(location, conf);
        }
    }
}