/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_err_pgs.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/21 14:22:22 by operez            #+#    #+#             */
/*   Updated: 2024/09/10 11:34:55 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void    err_pgs_to_conf(std::list<std::string> & err_pgs, t_conf & conf)
{
    std::pair<std::string, std::string> pair;

    for (std::list<std::string>::iterator it = ++err_pgs.begin(); it != err_pgs.end(); it++)
    {
        if (*it == "{" || *it == "}")
            continue ;
        pair.first = extract_index(*it);
        pair.second = extract_conf(*it, ';');
        conf.err_pgs.insert(make_pair(pair.first, pair.second));
    }
}

std::list<std::string>  extract_bloc_err_pgs(std::list<std::string>::iterator begin)
{
    std::list<std::string>::iterator    end;
    std::list<std::string>::iterator    it = begin;
    std::list<std::string>              err_pgs;

    while (1)
    {
        if ((*it).find('}') != (*it).npos)
        {
            end = it;
            break ;
        }
        it++;
    }
    err_pgs.insert(err_pgs.end(), begin, end);
    return (err_pgs);
}

void    handle_err_pgs(std::list<std::string> & cnf_file, t_conf & conf)
{
    std::list<std::string> err_pgs;
    
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        if ((*it).find("error_page {") != (*it).npos || (*it).find("error_page{") != (*it).npos)
        {
            err_pgs = extract_bloc_err_pgs(it);
            err_pgs_to_conf(err_pgs, conf);
        }
    }
}