/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conf_parsing.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 14:00:28 by operez            #+#    #+#             */
/*   Updated: 2024/06/04 14:00:24 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <map>
#include <sstream>
#include <list>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct s_conf
{
  std::vector<std::string>  ipv4_port;
  std::vector<std::string>  ipv6_port;
  std::string               server_name;
  std::vector<std::string>  root_dir;
  std::vector<std::string>  default_files;
  std::vector<std::string>  location;
}t_conf;

std::string extract_conf(std::string buff)
{
  int end = buff.find(' ');
  buff.erase(0, end + 1);
  end = buff.find(';');
  std::string tmp (buff.substr(0, end));
  buff.erase(0, end + 2);
  return (tmp);
}

int     print_error(char const *str)
{
    std::cerr << "Error: " << str << std::endl; 
    return (-1);
}
// void    init_conf_file(char *argv, t_conf & conf)
// {
//   conf.port = extract_conf(buff, ';');                   // a gerer plus efficacement
//   conf.server_name = extract_conf(buff, ';');
//   conf.root_dir = extract_conf(buff, ';');
//   conf.default_files = extract_conf(buff, ';');
// }

void    set_conf_struct(std::list<std::string> const & cnf_file)
{
    
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
        std::unique((*it).begin(), (*it).end(), is_white_space);      //delete duplicate
        if ((*it).find('#') == 0)
        {
            it = cnf_file.erase(it);
            continue ;
        }
        it++;
    }
}

int     parse_conf_file(char *argv)
{
    std::ifstream               file;
    std::string                 buff;
    std::list<std::string>      cnf_file;

    clear_file(cnf_file, argv);
    for (std::list<std::string>::iterator it = cnf_file.begin(); it != cnf_file.end(); it++)
    {
        std::cout << *it << std::endl;
    }
    set_conf_struct(cnf_file);
    // if (buff.empty())
        // return (print_error("Configuration file empty"));
    return (0);
}

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        if (parse_conf_file(argv[1]) == -1)
            return (1);
    }
    return (0);
}