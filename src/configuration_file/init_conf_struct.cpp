/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_conf_struct.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:37:23 by operez            #+#    #+#             */
/*   Updated: 2024/06/18 18:46:51 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

void    switch_flag(int & flag)
{
    if (flag == 0)
        flag = 1;
    else
        flag = 0;
}

std::string extract_conf(std::string buff, char c)
{
    unsigned long pos = buff.find(' ');
    buff.erase(0, pos + 1);
    pos = buff.find(c);
    if (pos == buff.npos)
    {
        if (buff.find_first_of("{}") == buff.npos)
            throw ConfFileException("Error: expected ';' at end of declaration");
    }
    std::string tmp (buff.substr(0, pos));
    return (tmp);
}

void    set_conf_struct(std::list<std::string> & cnf_file, t_conf & conf)
{
    // for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    // {
        // std::cout << *it << std::endl;
    // }
    int i = 0;
    int flag_loc = 0;                  // check if whether we are inside location or not (matter for root setting)
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        if (it != cnf_file.begin())
        {
            if ((*it).find("server{") != (*it).npos || (*it).find("server {") != (*it).npos|| (*it).find("server\0") != (*it).npos) 
            {
                handle_locations(cnf_file, conf);
                return ; 
            }
        }
        if ((*it).find("location ") != (*it).npos)
            switch_flag(flag_loc);
        if ((*it).find("}") != (*it).npos && flag_loc == 1)
            switch_flag(flag_loc);
        if ((*it).find("listen ") != (*it).npos)
        {
            if ((*it).find("[..]") != (*it).npos)
                 conf.ipv6_port.push_back(extract_conf(*it, ';'));
            else
                 conf.ipv4_port.push_back(extract_conf(*it, ';'));
        }
        if ((*it).find("root ")!= (*it).npos && flag_loc == 0)
            conf.root_dir = extract_conf(*it, ';');
        if ((*it).find("client_max_body_size ")!= (*it).npos)
            conf.max_body_size = extract_conf(*it, ';');
        if ((*it).find("index ")!= (*it).npos)
            conf.files = extract_conf(*it, ';');
        if ((*it).find("server_name ") != (*it).npos)
            conf.server_name = extract_conf(*it, ';');
        i++;
    }
    handle_locations(cnf_file, conf);
    // print_locations(conf);
}