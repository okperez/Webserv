/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_content.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 16:07:39 by operez            #+#    #+#             */
/*   Updated: 2024/09/10 11:33:49 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void    extract_returns_values(std::vector<t_conf> & conf, std::vector<std::string> & r_values)
{
    for (std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++)
    {
        if (!(*it).ret.empty())
            r_values.push_back((*it).ret);
        for (std::map<std::string, std::map<std::string, std::string> >::iterator its = (*it).location.begin(); its != (*it).location.end(); its++)
        {
            for (std::map<std::string, std::string>::iterator itt = (*its).second.begin(); itt != (*its).second.end(); itt++)
            {
                if ((*itt).first == "return")
                    r_values.push_back((*itt).second);
            }
        }
    }
}

void    check_returns(std::vector<t_conf> & conf)
{
    std::vector<std::string> r_values;
    std::map<int, std::string> map;
    extract_returns_values(conf, r_values);
    for (std::vector<std::string>::iterator it = r_values.begin(); it != r_values.end(); it++)
    {
        if ((*it).find(' ') == (*it).npos)
            throw ConfFileException ("Error: return syntax");
        fill_map(map, (*it));
    }
    for (std::map<int, std::string>::iterator it = map.begin(); it != map.end(); it++)
    {
        if ((*it).first < 300 || (*it).first > 308)
            throw ConfFileException ("Error: return value out of range");
        if ((*it).second.empty())
            throw ConfFileException ("Error: return directory non defined");
    }
}

void    check_for_root(std::map<std::string, std::map<std::string, std::string> > & location)
{
    int flag;
    for (std::map<std::string, std::map<std::string, std::string> >::iterator it = location.begin(); it != location.end(); it++)
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

void    check_if_valid(std::string str, int loc, int err_pgs)
{
    std::string arg = str.substr(0, str.find(' '));
    std::string valid_loc = "index allow_methods autoindex upload_dir root cgi_extension cgi_path return location try_files";
    std::string valid_server = "index root listen host location client_max_body_size error_page server{ }";
    std::string error_page = "200 300 301 302 303 304 305 306 307 308 400 403 404 405 406 411 413 415 500 501 503 505 508 error_page";                  // quelles sont les autres possiblites ?
    
    if (loc == 1)
    {
        if (valid_loc.find(arg) == valid_loc.npos)
        {
            
            throw ConfFileException ("Error: invalid argument in location");
        }
    }
    else if (err_pgs == 1)
    {
        if (error_page.find(arg) == error_page.npos)
        {
            throw ConfFileException ("Error: invalid argument in error_page");
        }
    }
    else if (loc == 0 && err_pgs == 0)
    {
        if (valid_server.find(arg) == valid_server.npos)
        {
            throw ConfFileException ("Error: invalid argument in server");
        }
    }
}

void    check_if_missing(t_conf & conf, std::list<std::string> & cnf_file)
{
    int location_methods = 0;
    int total_methods = 0;
    
    if (conf.ipv4_port.size() == 0)
        throw ConfFileException ("Error: missing port number");
    if (conf.max_body_size.empty())
        conf.max_body_size = DFLT_BODY_SIZE;
    if (conf.root_dir.empty())
        check_for_root(conf.location);
    if (conf.server_name.empty())
        conf.server_name = "";
    for (std::map<std::string, std::map<std::string, std::string> >::iterator it = conf.location.begin(); it != conf.location.end(); it++)
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