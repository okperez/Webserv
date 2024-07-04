/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_content.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 16:07:39 by operez            #+#    #+#             */
/*   Updated: 2024/07/03 15:46:17 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

char*        get_ip_address(void)
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        std::cerr << "Error getting hostname" << std::endl;
    }
    struct hostent *host_entry = gethostbyname(hostname);
    if (host_entry == NULL) {
        std::cerr << "Error getting host entry" << std::endl;
    }
    char *ip = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); // VOIR AVEC ORLANDO SI FONCTION AUTORISEE
    return (ip);
}

void    check_for_root(std::map<std::string, std::map<std::string, std::string>> & location)
{
    int flag;
    for (auto it = location.begin(); it != location.end(); it++)
    {
        flag = 0;
        for (auto its = (*it).second.begin(); its != (*it).second.end(); its++)
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
    std::string valid_server = "index root listen host client_max_body_size error_page server{ }";
    std::string error_page = "404 403 error_page";
    
    if (loc == 1)
    {
        if (valid_loc.find(arg) == valid_loc.npos)
        {
            
            std::cout << "Invalid arg = " << arg << std::endl;
            throw ConfFileException ("Error: invalid argument in location");
        }
    }
    else if (err_pgs == 1)
    {
        if (error_page.find(arg) == error_page.npos)
        {
            std::cout << "Invalid arg = " << arg << std::endl;
            throw ConfFileException ("Error: invalid argument in error_page");
        }
    }
    else if (loc == 0 && err_pgs == 0)
    {
        if (valid_server.find(arg) == valid_server.npos)
        {
            std::cout << "Invalid arg = " << arg << std::endl;
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
        throw ConfFileException ("Error: missing body size number");
    if (conf.root_dir.empty())
        check_for_root(conf.location);
    if (conf.files == "")
        conf.files = "index.html";
    if (conf.server_name.empty())
        conf.server_name = get_ip_address();
    for (auto it = conf.location.begin(); it != conf.location.end(); it++)
    {
        for(auto its = (*it).second.begin(); its != (*it).second.end(); its++)
        {
            if (is_allow_methods((*its).first))
                location_methods++;
        }
    }
    total_methods = std::count_if(cnf_file.begin(), cnf_file.end(), is_allow_methods);
    if (total_methods > location_methods)
        throw ConfFileException ("Error: allow_methods present outside location");
 }