/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 14:00:36 by operez            #+#    #+#             */
/*   Updated: 2024/06/28 17:50:01 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

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

void	send_response(int socket_fd, t_request &request, std::string &response_content) {
	
	std::stringstream stream;
	std::string       tmp;
	
	request.lenght = request.body.size();
	// std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	stream << request.lenght;
	stream >> tmp;
	response_content += tmp + "\r\n\r\n" + request.body;
	write(socket_fd, response_content.c_str(), response_content.size());
}

void	build_response(int socket_fd, t_request &request, t_conf &conf, std::map<std::string, std::string> map_error) {
	
	std::ifstream     file;
	std::string response_content;

	file.open(request.target);
	if (file.is_open()) { //GERER ERREUR si pas bon 
		std::getline(file, request.body, '\0');
		// a verifier text/html si css jss ou utilise avec curl ou http...
		response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	}
	else
		fill_error(request.body, response_content, "404", conf, map_error);
	send_response(socket_fd, request, response_content);
}

//  parse request from client and send back response 
int  handle_request(int socket_fd, t_request & request, t_conf & conf, std::map<std::string, std::string> map_error)
{
	// /* A EFFACER UNE FOIS PARSING DONE */
	conf.err_pgs["404"] = "pages/error_pages/error_404.html";
	conf.err_pgs["405"] = "pages/error_pages/error_405.html";
	// /* ********************************* */

	std::string index = look_for_location(request.target, conf); // si pas trouve index = ""
	// std::cout << "index = " << index << std::endl;
	if (index.empty())
	{
		if (conf.root_dir.empty()) {
			std::string response;
			fill_error(request.body, response, "404", conf, map_error);
			return (send_response(socket_fd, request, response), 1);
		}
		else {
			if (conf.root_dir.back() == '/')
				request.target.replace(0, 1, conf.root_dir);
			else	
				request.target.insert(0, conf.root_dir);
		}
	}
	/* AVANT D AJOUTER LE PATH => check method ok */
	else {
		if (!check_allow_method(request, conf, index)) {
			std::string response;
			fill_error(request.body, response, "405", conf, map_error);
			return (send_response(socket_fd, request, response), 1);
		}
		add_path(request.target, conf, index); // GET POST DELETE
	}
	// std::cout << "request.target " << request.target << std::endl;
	return (build_response(socket_fd, request, conf, map_error), 1); // GET ONLY ICI
}

// TEST OK
/* Si plusieurs serveurs, retourne l index du fichier conf correspondant au server, sinon 0 par default */
int  choose_server_curl(t_request & request, std::vector<t_conf> & conf) {
  
  std::istringstream	iss(request.host);
  std::string         	host;
  std::string         	port;
  int					i = 0;

  std::getline(iss, host, ':');
  std::getline(iss, port);
  std::cout << "host = " << host << std::endl;
  std::cout << "port = " << port << std::endl;
  for (std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
	if (host == it->server_name) {
		std::vector<std::string>::iterator it = find(conf[i].ipv4_port.begin(), conf[i].ipv4_port.end(), port);
		if (it != conf[i].ipv4_port.end())
			return (i);
	}
	i++;  
  }
  return (0);
}

// TEST OK
/* Si plusieurs serveurs, retourne l index du fichier conf correspondant au server, sinon 0 par default */
int  choose_server_browser(t_request & request, std::vector<t_conf> & conf) {
  
  std::istringstream	iss(request.host);
  std::string         	host;
  std::string         	port;
  int					i = 0;

  std::getline(iss, host, ':');
  std::getline(iss, port);
  std::cout << "host = " << host << std::endl;
  std::cout << "port = " << port << std::endl;
  for (std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
	if (host == it->host || !it->host.length()) {
		std::vector<std::string>::iterator it = find(conf[i].ipv4_port.begin(), conf[i].ipv4_port.end(), port);
		if (it != conf[i].ipv4_port.end())
			return (i);
	}
	i++;  
  }
  return (0);
}

/* Reste a checker si le port est bien present dans le serveur qu on retourne */
int	is_host(std::string host, std::string port, std::vector<t_conf> &conf) {
	int i = 0;
	
	for (std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
		if (host == it->host || (host == "localhost" && it->host == "127.0.0.1")) {
			for (std::vector<std::string>::iterator jt = it->ipv4_port.begin(); jt != it->ipv4_port.end(); jt++)
				if (*jt == port)
					return (i);
		}
		i++;
	}
	return (-1);
}

/* Reste a checker si le port est bien present dans le serveur qu on retourne */
int	is_server_name(std::string host, std::string port, std::vector<t_conf> &conf) {
	int i = 0;
	
	for (std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
		if (host == it->server_name) {
			for (std::vector<std::string>::iterator jt = it->ipv4_port.begin(); jt != it->ipv4_port.end(); jt++)
				if (*jt == port)
					return (i);
		i++;
		}
	}
	return (-1);
}
/*
URI Comparison

   When comparing two URIs to decide if they match or not, a client
   SHOULD use a case-sensitive octet-by-octet comparison of the entire
   URIs, with these exceptions:

      - A port that is empty or not given is equivalent to the default
        port for that URI-reference;

        - Comparisons of host names MUST be case-insensitive;

        - Comparisons of scheme names MUST be case-insensitive;

        - An empty abs_path is equivalent to an abs_path of "/".

   Characters other than those in the "reserved" and "unsafe" sets (see
   RFC 2396 [42]) are equivalent to their ""%" HEX HEX" encoding.

   For example, the following three URIs are equivalent:

      http://abc.com:80/~smith/home.html
      http://ABC.com/%7Esmith/home.html
      http://ABC.com:/%7esmith/home.html

*/
int	pick_server(t_request &request, std::vector<t_conf> &conf) {
	std::istringstream	iss(request.host);
	std::string         	host;
	std::string         	port;
	
	std::getline(iss, host, ':');
	std::getline(iss, port);
	std::cout << "host = " << host << std::endl;
	std::cout << "port = " << port << std::endl;
	
	if (conf.size() == 0)
		return (0);
	std::cout << "is_host = " << is_host(host, port, conf) << std::endl;
	if (is_host(host, port, conf) > -1) { 
		std::cout << "true" << std::endl;
	}
	else
		std::cout << "false" << std::endl;
	return (0);
}

/*
The normal procedure for parsing an HTTP message is to read the start-line
into a structure, read each header field line into a hash table by field name
until the empty line, and then use the parsed data to determine if a message
body is expected. If a message body has been indicated, then it is read as a
stream until an amount of octets equal to the message body length is read or the
connection is closed.
*/
void	do_request(struct pollfd *fds, int i, char *buffer, std::vector<t_conf> & conf, std::map<std::string, std::string> map_error) {

	t_request 	request;
	int			i_conf = 0;
	
	// for (int j = 0; j < 1024; j++)
	// 	std::cout << buffer[j];
	// std::cout << std::endl;
	init_request_struct(request, buffer);
  	// print_request(request);
  /*
  revoir choose_server avec info de ce site :
  https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms
  */
  // est ce qu on verifie que le port est bien ecoute + meme serveur name + meme host?

  
	i_conf = pick_server(request, conf);

	
	// if (request.agent.substr(0, 4) == "curl" && conf.size() > 1)
	// 	i_conf = choose_server_curl(request, conf);
	// else
	// 	i_conf = choose_server_browser(request, conf);
	handle_request(fds[i].fd, request, conf[i_conf], map_error);
}
