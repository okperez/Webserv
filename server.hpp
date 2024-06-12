/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:03:27 by operez            #+#    #+#             */
/*   Updated: 2024/06/11 15:55:28 by galambey         ###   ########.fr       */
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

typedef struct s_conf
{
  std::string               ipv4_port;
  std::string               ipv6_port;
  std::string               server_name;
  std::string               root_dir;
  std::string               files;
  std::string               location;
}t_conf;

class   ConfFileException : public std::exception
{
    public:
    
    char const * s;
    ConfFileException(char const* str) : s(str) {}
    virtual const char* what() const throw()
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
  std::string   agent;    // Header that describes the client's user agent
  std::string   media;    // Header that specifies which media types the client can accept

  // BODY
  std::string   body;
  std::string   content_type;
  size_t        lenght;

}t_request;

void    init_request_struct(t_request & request, char const *buffer);
int     handle_request(int socket_fd, t_request & request);
int     parse_conf_file(char *argv);

#endif