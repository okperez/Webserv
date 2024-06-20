/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:03:27 by operez            #+#    #+#             */
/*   Updated: 2024/06/20 12:03:48 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <fstream>
# include <cstdlib>
# include <string>
# include <cstring>
# include <algorithm>
# include <vector>
# include <map>
# include <list>
# include <sstream>
# include <vector>
#include <utility>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>

#include "ServerException.hpp"

#define MAX_CONNECTION	20

typedef struct s_conf
{
  std::vector<std::string>                                          ipv4_port;
  std::vector<std::string>                                          ipv6_port;
  std::string														                            max_body_size;
  std::string														                            host;
  std::string                                                       server_name;
  std::string                                                       root_dir;
  std::string                                                       files;
  std::map<std::string, std::map<std::string, std::string>>         location;
  std::map<std::string, std::string>										err_pgs;
}t_conf;

// A EFFACER ET REMPLACER PAR LE VRAI T_CONF UNE FOIS PARSING DONE
typedef struct s_conftest
{
  std::vector<std::string>  ipv4_port;
  std::string               ipv6_port; // vecteur
  std::string               server_name;
  std::string               root_dir;
  std::string               files;
  std::string               location; // map<string, map<string(root, index, method, nom page), string >>
}t_conftest;

class   ConfFileException : public std::exception
{
    public:
    
    char const * s;
    ConfFileException(char const* str) : s(str) {}
    virtual const char* what() const throw() // Attention def fonction dans .hpp
    {
        return (s);
    }
};

typedef struct s_request
{
  // REQUEST line
  std::string   method;   // HTTP method (ex: GET)
  std::string   target;   // Request target (ex: index.html)
  std::string   version;  // HTTP version (ex: HTTP/1.1)


  // HEADER section
  std::string   host;     // Header that specifies the server's host and port
  std::string   port;     // Header that describes the pport used
  std::string   agent;    // Header that describes the client's user agent
  std::string   media;    // Header that specifies which media types the client can accept

  // BODY
  std::string   body;
  std::string   content_type;
  size_t        lenght;

} t_request;

typedef struct s_listen
{
	int			fd;
	int			port;
	
} t_listen;

void    		init_request_struct(t_request & request, char const *buffer);
int     		handle_request(int socket_fd, t_request & request);
int         handle_conf_file(char *argv, std::vector<t_conf> & conf);
void        check_bracket(std::list<std::string> & cnf_file);
void        check_syntax(std::list<std::string> & cnf_file);
void        set_conf_struct(std::list<std::string> & cnf_file, t_conf & conf);
std::string extract_conf(std::string buff, char c);
void        handle_locations(std::list<std::string> & cnf_file, t_conf & conf);

/* ****************************** utils.cpp ******************************** */

void        clear_space(std::string & type);
bool        is_equal(t_conf & conf1, t_conf & conf2);
void        print_all_struct(std::vector<t_conf> & conf, int count);
std::string clear_str(std::list<std::string> cnf_file);
bool        is_allow_methods(std::string str);
int         print_error(char const *str);
bool        is_white_space(char c, char d);
void        clear_space(std::string & type);

/* ****************************** request.cpp ******************************** */

void	      	do_request(struct pollfd *fds, int i, char *buffer, std::vector<t_conf> & conf, std::map<std::string, std::string> map_error);
int     		handle_request(int socket_fd, t_request & request, t_conf & conf, std::map<std::string, std::string> map_error);
void    		init_request_struct(t_request & request, char const *buffer);

/* ****************************** server.cpp ******************************** */

void				open_listen_socket(std::vector<t_conf> &conf, std::vector<t_listen> &server_fd);
struct pollfd 		*create_fds(std::vector<t_listen> &server_fd);
void				launch_server(struct pollfd *fds, std::vector<t_listen> &server_fd, int max_socket, std::vector<t_conf> & conf);

/* *************************** close_server.cpp ***************************** */

void				close_connection(struct pollfd *fds, int i);
void				close_fds(struct pollfd *fds, int nb);
void				save_fds(struct pollfd *fds, int max);
void 				sighandler(int signal);

/* ********************************** ERROR ********************************* */
void	create_map_error(std::map<std::string, std::string> &map_error);
void	fill_error(std::string &body, std::string &response, std::string code, t_conf &conf, std::map<std::string, std::string> map_error);

/* ****************************** A_EFFACER ********************************* */

void    print_request(t_request & request);
void    print_location(std::vector<t_conf> &conf);
/* ************************************************************************** */
/* ********************************** ENUM ********************************** */
/* ************************************************************************** */

enum	e_rule {
	SAVE,
	CLOSE,
};

#endif