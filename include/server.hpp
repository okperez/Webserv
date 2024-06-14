/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:03:27 by operez            #+#    #+#             */
/*   Updated: 2024/06/14 18:10:29 by galambey         ###   ########.fr       */
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
  std::string               ipv4_port; // vecteur => string ou vector int a la fin du parsing?
  std::string               ipv6_port; // vecteur
  std::string               server_name;
  std::string               root_dir;
  std::string               files;
  std::string               location; // map<string, map<string(root, index, method, nom page), string >>
}t_conf;

// A EFFACER ET REMPLACER PAR LE VRAI T_CONF UNE FOIS PARSING DONE
typedef struct s_conftest
{
  std::vector<std::string>          ipv4_port;
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

void    			init_request_struct(t_request & request, char const *buffer);
int     			handle_request(int socket_fd, t_request & request);
int     			parse_conf_file(char *argv);
void        		check_bracket(std::list<std::string> & cnf_file);
void        		check_syntax(std::list<std::string> & cnf_file);

/* *************************** handle_gate.cpp ****************************** */

void				open_listen_socket(std::vector<t_conf> &conf, std::vector<t_listen> &server_fd);

/* ****************************** server.cpp ******************************** */

struct pollfd 		*create_fds(std::vector<t_listen> &server_fd);
void				launch_server(struct pollfd *fds, std::vector<t_listen> &server_fd, int max_socket);

/* *************************** close_server.cpp ***************************** */

void				close_connection(struct pollfd *fds, int i);
void				close_fds(struct pollfd *fds, int nb);
void				save_fds(struct pollfd *fds, int max);
void 				sighandler(int signal);

/* ************************************************************************** */
/* ********************************** ENUM ********************************** */
/* ************************************************************************** */

enum	e_rule {
	SAVE,
	CLOSE,
};

#endif