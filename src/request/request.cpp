/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 14:00:36 by operez            #+#    #+#             */
/*   Updated: 2024/06/18 19:46:36 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"

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

/*
Recherche pour la location de la plus precise a la moins precise dans le serveur
*/
std::string look_if_location(std::string &target, t_conf & conf) {

	std::cout << "Look_for_location avec target = " << target << std::endl;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(target);
	if (it != conf.location.end()) {
		std::cout << "FIND : " << it->first << std::endl;
		return (it->first);	
	}
	std::string s = target;
	if (target.back() == '/')
		s.erase(s.length() - 1, 1); 
	else {
		size_t found = s.rfind('/');
		if (found == std::string::npos) {
			std::cout << "NOT FOUND" << std::endl; // si not found + pas de root globale pour le serveur => page d erreur en dur 
			return ("");
		}
		s.erase(found + 1, s.length() - found + 1);
	}
	return (look_if_location(s, conf));
}

/*
Cherche si une location = correspondante a la target existe, sinon appelle look_if_location
*/
std::string look_for_location(std::string &target, t_conf & conf) {
	
	std::string s = "=" + target;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	std::cout << "hey" << std::endl;
	return (look_if_location(target, conf));
	//on recupere l index de la location :)
}

/* Supprime la location et la remplace par la root */
void	add_path(std::string &target, t_conf & conf, std::string &index) {
	
	std::string root;
	
	if (conf.location[index].find("root") == conf.location[index].end())
		root = conf.root_dir;
	else
		root  = conf.location[index]["root"];
	// else if(index.back() == '/' && conf.location[index]["root"].back() != '/')
	// 	conf.location[index]["root"] += "/";
	if(index.back() == '/' && root.back() != '/')
		root += "/";
	target.replace(target.find(index), index.length(), root);
	std::cout << "target : " << target << std::endl;
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
	// std::stringstream stream;
	// std::string       tmp;
	std::string response_content;

	file.open(request.target);
	if (file.is_open()) { //GERER ERREUR si pas bon 
		std::getline(file, request.body, '\0');
		response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	}
	else
		fill_error(request.body, response_content, "404", conf, map_error);
	send_response(socket_fd, request, response_content);
	// request.lenght = request.body.size();
	// // std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	// stream << request.lenght;
	// stream >> tmp;
	// response_content += tmp + "\r\n\r\n" + request.body;
	// write(socket_fd, response_content.c_str(), response_content.size());
}

//  parse request from client and send back response 
int  handle_request(int socket_fd, t_request & request, t_conf & conf, std::map<std::string, std::string> map_error)
{
	// const char *response_true = "HTTP/1.1 200 OK\r\n\r\n";
	// const char *response_false = "HTTP/1.1 404 Not Found\r\n\r\n";

	/* A EFFACER UNE FOIS PARSING DONE */
	std::map<std::string, std::string> tmp1;
	tmp1["root"] = "pages";
	conf.location["/lol"] = tmp1; 
	conf.location["=/lol/"] = tmp1; 
	// conf.location["=/lol/index.html"] = tmp1; => a voir comment on traite la root
	// conf.location["/"] = tmp1;
	conf.err_pgs["404"] = "pages/error_pages/error_404.html";
	// conf.root_dir = "";
	/* ********************************* */

	std::string index = look_for_location(request.target, conf); // si pas trouve index = ""
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
		// write(socket_fd, response_true, strlen(response_true));
		// return (1);
	}
	/* AVANT D AJOUTER LE PATH => check method ok */
	else
		add_path(request.target, conf, index); // GET POST DELETE
	std::cout << "request.target " << request.target << std::endl;
	return (build_response(socket_fd, request, conf, map_error), 1); // GET ONLY ICI

	
	// std::ifstream     file;
	// std::stringstream stream;
	// std::string       tmp;

	// file.open(request.target);
	// if (file.is_open()) //GERER ERREUR si pas bon
	// 	std::getline(file, request.body, '\0');
	// request.lenght = request.body.size();
	// std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	// stream << request.lenght;
	// stream >> tmp;
	// response_content += tmp + "\r\n\r\n" + request.body;
	// write(socket_fd, response_content.c_str(), response_content.size());
	// return (1);
		
	// if (request.target == "/")
	// {
	// 	write(socket_fd, response_true, strlen(response_true));
	// 	return (1);
	// }
	// else if (request.target == "/index.html")
	// {
		// std::ifstream     file;
		// std::stringstream stream;
		// std::string       tmp;

		// file.open("index.html");
		// if (file.is_open())
		// 	std::getline(file, request.body, '\0');
		// request.lenght = request.body.size();
		// std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
		// stream << request.lenght;
		// stream >> tmp;
		// response_content += tmp + "\r\n\r\n" + request.body;
		// write(socket_fd, response_content.c_str(), response_content.size());
		// return (1);
	// }
	// else if (request.target.substr(0, request.target.rfind('/')) == "/echo")
	// {
	// 	std::string print = request.target.substr(request.target.rfind('/') + 1, request.target.find('\0') -1);
	// 	std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";

	// 	std::stringstream stream;
	// 	std::string       tmp;
	// 	request.lenght = print.size();
	// 	stream << print.size();
	// 	stream >> tmp;
	// 	response_content += tmp + "\r\n\r\n" + print;
	// 	request.body = response_content;
	// 	write(socket_fd, request.body.c_str(), request.body.size());
	// 	return (1);
	// }
	// else if ((request.target == "/user-agent"))
	// {
	// 	std::stringstream stream;
	// 	std::string       tmp;

	// 	request.body = request.agent;
	// 	request.lenght = request.body.size();
	// 	std::string response_content = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ";
	// 	stream << request.lenght;
	// 	stream >> tmp;
	// 	response_content += tmp + "\r\n\r\n" + request.body;
	// 	write(socket_fd, response_content.c_str(), response_content.size());
	// 	return (1);
	// }
	// write(socket_fd, response_false, strlen(response_false));
	// return (0);
}

// TEST OK
/* Si plusieurs serveurs, retourne l index du fichier conf correspondant au server, sinon 0 par default */
int  choose_server(t_request & request, std::vector<t_conf> & conf) {
  
  std::istringstream	iss(request.host);
  std::string         	host;
  int					i = 0;

  std::getline(iss, host, ':');
  for (std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
	if (host == it->server_name)
		return (i);
	i++;  
  }
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
	
	for (int j = 0; j < 1024; j++)
		std::cout << buffer[j];
	std::cout << std::endl;
	init_request_struct(request, buffer);
  	print_request(request);
  /*
  revoir choose_server avec info de ce site :
  https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms
  */
  // est ce qu on verifie que le port est bien ecoute + meme serveur name + meme host?
	if (request.agent.substr(0, 4) == "curl" && conf.size() > 1)
		i_conf = choose_server(request, conf);
	handle_request(fds[i].fd, request, conf[i_conf], map_error);
}
