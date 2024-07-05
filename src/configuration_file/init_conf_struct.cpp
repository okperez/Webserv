/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_conf_struct.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 10:37:23 by operez            #+#    #+#             */
/*   Updated: 2024/07/04 15:45:54 by operez           ###   ########.fr       */
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

void    handle_return(std::string str, t_conf & conf)
{
    if (conf.flag.ret == 1)
        return ;
    conf.ret = extract_conf(str, ';');
    conf.flag.ret = 1;
}

void    handle_files(std::string str, t_conf & conf)
{
    str.erase(0, str.find(' ') + 1);
    while (str.find(' ') != str.npos)
    {
        conf.files_vect.push_back(str.substr(0, str.find(' ')));
        str.erase(0, str.find(' ') + 1);
    }
    conf.files_vect.push_back(str.substr(0, str.find(';')));
    // for (auto it = conf.files_vect.begin(); it != conf.files_vect.end(); it++)
    // {
    //     std::cout << *it << std::endl;
    // }
}

void    set_flag(t_flag & flag)
{
    int loc = 0; 
    int err_pgs = 0;
    int ret = 0;
    int ret_loc = 0;
}

void    set_conf_struct(std::list<std::string> & cnf_file, t_conf & conf)
{
    int i = 0;

    set_flag(conf.flag);
    for (auto it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        set_flags(*it, conf.flag.loc, conf.flag.err_pgs);
        if ((*it).find("listen ") != (*it).npos)
        {
            if ((*it).find("[..]") != (*it).npos)
                 conf.ipv6_port.push_back(extract_conf(*it, ';'));
            else
                 conf.ipv4_port.push_back(extract_conf(*it, ';'));
        }
        else if ((*it).find("root ")!= (*it).npos && conf.flag.loc == 0)
            conf.root_dir = extract_conf(*it, ';');
        else if ((*it).find("client_max_body_size ")!= (*it).npos && conf.flag.loc == 0 && conf.flag.err_pgs == 0)
            conf.max_body_size = extract_conf(*it, ';');
        else if ((*it).find("index ")!= (*it).npos && conf.flag.loc == 0 && conf.flag.err_pgs == 0)
            handle_files((*it), conf);
        else if ((*it).find("server_name ") != (*it).npos && conf.flag.loc == 0 && conf.flag.err_pgs == 0)
            conf.server_name = extract_conf(*it, ';');
        else if ((*it).find("autoindex ") != (*it).npos && conf.flag.loc == 0 && conf.flag.err_pgs == 0)
            conf.autoindex = extract_conf(*it, ';');
        else if ((*it).find("host ") != (*it).npos && conf.flag.loc == 0 && conf.flag.err_pgs == 0)
            conf.host = extract_conf(*it, ';');
        else if ((*it).find("return ") != (*it).npos && conf.flag.loc == 0 && conf.flag.err_pgs == 0)
            handle_return((*it), conf);
        else
            check_if_valid((*it), conf.flag.loc, conf.flag.err_pgs);
        i++;
    }
    handle_locations(cnf_file, conf);
    handle_err_pgs(cnf_file, conf);
}

// si host absent host = 0.0.0.0
