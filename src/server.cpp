/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 14:00:36 by operez            #+#    #+#             */
/*   Updated: 2024/06/03 14:11:43 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <map>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef struct s_conf
{
  std::string port;
  std::string server_name;
  std::string root_dir;
  std::string default_files;
}t_conf;

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

std::string extract_line(std::string & buff, char delim)
{
  int end = buff.find(delim);
  std::string tmp (buff.substr(0, end));
  if (delim == '\r')
    end += 1;
  buff.erase(0, end + 1);
  return(tmp);
}

std::string extract_header(std::string & buff, char delim)
{
  int end = buff.find(' ');
  buff.erase(0, end + 1);
  end = buff.find('\r');
  std::string tmp (buff.substr(0, end));
  buff.erase(0, end + 2);
  return (tmp);
}

void        init_request_struct(t_request & request, char const *buffer)
{
  std::string buff = buffer;

  request.method = extract_line(buff, ' ');
  request.target = extract_line(buff, ' ');
  request.version = extract_line(buff, '\r');
  request.host = extract_header(buff, '\r');
  request.agent = extract_header(buff, '\r');
  request.media = extract_header(buff, '\r');
}

int  handle_request(int socket_fd, char *buffer, t_request & request)
{
  const char *response_true = "HTTP/1.1 200 OK\r\n\r\n";
  const char *response_false = "HTTP/1.1 404 Not Found\r\n\r\n";

  if (request.target == "/")
  {
    write(socket_fd, response_true, strlen(response_true));
    return (1);
  }
  else if (request.target == "/index.html")
  {
    std::ifstream     file;
    std::stringstream stream;
    std::string       tmp;

    file.open("index.html");
    if (file.is_open())
      std::getline(file, request.body, '\0');
    request.lenght = request.body.size();
    std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
    stream << request.lenght;
    stream >> tmp;
    response_content += tmp + "\r\n\r\n" + request.body;
    write(socket_fd, response_content.c_str(), response_content.size());
    return (1);
  }
  else if (request.target.substr(0, request.target.rfind('/')) == "/echo")
  {
    std::string print = request.target.substr(request.target.rfind('/') + 1, request.target.find('\0') -1);
    std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";

    std::stringstream stream;
    std::string       tmp;
    request.lenght = print.size();
    stream << print.size();
    stream >> tmp;
    response_content += tmp + "\r\n\r\n" + print;
    request.body = response_content;
    write(socket_fd, request.body.c_str(), request.body.size());
    return (1);
  }
  else if ((request.target == "/user-agent"))
  {
    std::stringstream stream;
    std::string       tmp;

    request.body = request.agent;
    request.lenght = request.body.size();
    std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
    stream << request.lenght;
    stream >> tmp;
    response_content += tmp + "\r\n\r\n" + request.body;
    write(socket_fd, response_content.c_str(), response_content.size());
    return (1);
  }
  write(socket_fd, response_false, strlen(response_false));
  return (0);
}


int main(int argc, char **argv) 
{
  // if (argc == 2)
  // {
    t_request request;
    t_conf    conf;

    // init_conf_file(argv[1], conf);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);    //socket set up for listening is used only for accepting connections, not for exchanging data
    if (server_fd < 0) {
     std::cerr << "Failed to create server socket\n";
     return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;           // address family
    server_addr.sin_addr.s_addr = INADDR_ANY;   //The address for this socket. This is just your machine’s IP address
    server_addr.sin_port = htons(4221);         //The port number (the transport address)

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) { // assigning a transport address to the socket
      std::cerr << "Failed to bind to port 4221\n";
      return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {   //The listen system call tells a socket that it should be capable of accepting incoming connections:
      std::cerr << "listen failed\n";                   //backlog, defines the maximum number of pending connections that can be queued up before connections are refused.
      return 1;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";
    //he accept system call grabs the first connection request on the queue of pending connections (set up in listen) and creates a new socket for that connection.
    //By default, socket operations are synchronous, or blocking, and accept will block until a connection is present on the queue.
    int socket_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    if (socket_fd < 0)
    {
      std::cerr << "Failed to connect\n";
      return (1);
    }
    std::cout << "Client connected\n";

//  --------------------------------------------------------------------
//  This part has successfully created TCP socket on the server computer
//  --------------------------------------------------------------------


    char buffer[1024] = {0};
    read(socket_fd, buffer, 1024);

    /*The normal procedure for parsing an HTTP message is to read the start-line
    into a structure, read each header field line into a hash table by field name
    until the empty line, and then use the parsed data to determine if a message
    body is expected. If a message body has been indicated, then it is read as a
    stream until an amount of octets equal to the message body length is read or the
    connection is closed.*/

    init_request_struct(request, buffer);
    // std::cout << "buffer = " << buffer;
    handle_request(socket_fd, buffer, request);
    close(server_fd);
    close(socket_fd);

  // }
  std::cerr << "Error: Missing configuration file: ./webserv [configuration file]\n";
  return 0;
}
