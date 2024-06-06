/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 14:00:36 by operez            #+#    #+#             */
/*   Updated: 2024/06/05 18:04:11 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server.hpp"

std::string extract_line(std::string & buff, char delim)
{
  int end = buff.find(delim);
  std::string tmp (buff.substr(0, end));
  if (delim == '\r')
    end += 1;
  buff.erase(0, end + 1);
  return(tmp);
}

std::string extract_header(std::string & buff)
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
  request.host = extract_header(buff);
  request.agent = extract_header(buff);
  request.media = extract_header(buff);
}

//  parse request from client and send back response 

int  handle_request(int socket_fd, t_request & request)
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
