/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_location.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 11:14:34 by operez            #+#    #+#             */
/*   Updated: 2024/06/20 13:52:47 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

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
    int                                 size = location.size();
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
    // for (std::map<std::string, std::string>::iterator it = path.begin(); it != path.end(); it++)
    // {
        // std::cout << "Map content = " << (*it).first << " | "  << (*it).second << std::endl;
    // }
    // std::cout << std::endl;
    conf.location.insert(make_pair(type, path));
}

std::list<std::string>  extract_bloc_location(std::list<std::string> & cnf_file, std::list<std::string>::iterator begin)
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
        it++;
    }
    location.insert(location.end(), begin, end);
    // std::cout << "Bloc location =\n";
    // for (std::list<std::string>::iterator it = location.begin(); it != location.end(); it++)
    // {
        // std::cout << *it << std::endl;
    // }
    return (location);
}

void    handle_locations(std::list<std::string> & cnf_file, t_conf & conf)
{
    std::list<std::string> location;
    
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        // std::cout << "STRING HERE ========== " << *it << std::endl;
        // if (it != cnf_file.begin())
        // {
            // if ((*it).find("server{") != (*it).npos || (*it).find("server {") != (*it).npos|| (*it).find("server ") != (*it).npos) 
            // {
                // break ;
            // }
        // }
        if ((*it).find("location") != (*it).npos)
        {
            location = extract_bloc_location(cnf_file, it);
            location_to_conf(location, conf);
        }
    }
}

// void    print_locations(t_conf & conf)
// {
    // std::cout << "Starts here" << std::endl;
    // for (std::map<std::string, std::map<std::string, std::string>>::iterator it = conf.location.begin(); it != conf.location.end(); it++)
    // {
        // std::cout << "Location " << (*it).first << std::endl;
        // for (std::map<std::string, std::string>::iterator its = (*it).second.begin(); its != (*it).second.end(); its++)
        // {
            // std::cout << (*its).first << " | " << (*its).second << std::endl;
        // }
        // std::cout << std::endl;
    // }
// }