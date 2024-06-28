/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_conf_struct.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:37:23 by operez            #+#    #+#             */
/*   Updated: 2024/06/28 17:50:01 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

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

void    set_flags(std::string str, int & flag_loc, int & flag_err_pgs)
{
    if (str.find("location ") != (str).npos)
        switch_flag(flag_loc);
    else if ((str).find("}") != (str).npos && flag_loc == 1)
        switch_flag(flag_loc);
    if ((str).find("error_page ") != (str).npos)
        switch_flag(flag_err_pgs);
    else if ((str).find("}") != (str).npos && flag_err_pgs == 1)
        switch_flag(flag_err_pgs);
}

void    set_conf_struct(std::list<std::string> & cnf_file, t_conf & conf)
{
    // for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    // {
        // std::cout << *it << std::endl;
    // }
    int i = 0;
    int flag_loc = 0;                  // check if whether we are inside location or not (matter for root setting)
    int flag_err_pgs = 0;
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        set_flags(*it, flag_loc, flag_err_pgs);
        if ((*it).find("listen ") != (*it).npos)
        {
            if ((*it).find("[..]") != (*it).npos)
                 conf.ipv6_port.push_back(extract_conf(*it, ';'));
            else
                 conf.ipv4_port.push_back(extract_conf(*it, ';'));
        }
        else if ((*it).find("root ")!= (*it).npos && flag_loc == 0)
            conf.root_dir = extract_conf(*it, ';');
        else if ((*it).find("client_max_body_size ")!= (*it).npos && flag_loc == 0 && flag_err_pgs == 0)
            conf.max_body_size = extract_conf(*it, ';');
        else if ((*it).find("index ")!= (*it).npos && flag_loc == 0 && flag_err_pgs == 0)
            conf.files = extract_conf(*it, ';');
        else if ((*it).find("server_name ") != (*it).npos && flag_loc == 0 && flag_err_pgs == 0)
            conf.server_name = extract_conf(*it, ';');
        else if ((*it).find("host ") != (*it).npos && flag_loc == 0 && flag_err_pgs == 0)
            conf.host = extract_conf(*it, ';');
        else
            check_if_valid((*it), flag_loc, flag_err_pgs);
        i++;
    }
    handle_locations(cnf_file, conf);
    handle_err_pgs(cnf_file, conf);
    // print_locations(conf);
}