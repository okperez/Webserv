/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_conf_struct.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:37:23 by operez            #+#    #+#             */
/*   Updated: 2024/06/14 15:25:37 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

std::string extract_name(std::string buff)
{
    std::string name = buff.substr(0, buff.find(' '));
    return (name);    
}

std::string extract_conf(std::string buff)
{
    unsigned long pos = buff.find(' ');
    buff.erase(0, pos + 1);
    pos = buff.find(';');
    if (pos == buff.npos)
    {
        if (buff.find_first_of("{}") == buff.npos)
            throw ConfFileException("Error: expected ';' at end of declaration");
    }
    std::string tmp (buff.substr(0, pos));
    return (tmp);
}

void    init_conf_location(std::list<std::string> & location, t_conf & conf)
{
    int         size = location.size();
    std::pair<std::string, std::string> pair;
    std::map<std::string, std::string>  path;
    std::string                         type;

    type = extract_conf(*(location.begin()));
    
    for (std::list<std::string>::iterator it = ++location.begin(); it != location.end(); it++)
    {
        // std::cout << "Before add to pair = " << *it << std::endl;
        pair.first = extract_name(*it);
        pair.second = extract_conf(*it);
        path[pair.first] = pair.second;
        
    }
    // for (std::map<std::string, std::string>::iterator it = path.begin(); it != path.end(); it++)
    // {
        // std::cout << "Map content = " << (*it).first << " | "  << (*it).second << std::endl;
    // }
    // std::cout << std::endl;
    // conf.location[type] = path;
    conf.location.insert(make_pair(type, path));
}

std::list<std::string>  extract_location(std::list<std::string> & cnf_file, std::list<std::string>::iterator begin)
{
    std::list<std::string>::iterator end;
    std::list<std::string>::iterator it = begin;
    std::list<std::string> location;

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
    // std::cout << "Bloc notation =\n";
    // for (std::list<std::string>::iterator it = location.begin(); it != location.end(); it++)
    // {
        // std::cout << *it << std::endl;
    // }
    return (location);
}

void    parse_locations(std::list<std::string> & cnf_file, t_conf & conf)
{
    std::list<std::string> location;
    
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        if ((*it).find("location") != (*it).npos)
        {
            location = extract_location(cnf_file, it);
            init_conf_location(location, conf);
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

void    set_conf_struct(std::list<std::string> & cnf_file, t_conf & conf)
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
        if ((*it).find("root ")!= (*it).npos)
            conf.root_dir = extract_conf(*it);
        if ((*it).find("index ")!= (*it).npos)
            conf.files = extract_conf(*it);
        if ((*it).find("server_name ") != (*it).npos)
            conf.server_name = extract_conf(*it);
    }
    parse_locations(cnf_file, conf);
    // print_locations(conf);

}