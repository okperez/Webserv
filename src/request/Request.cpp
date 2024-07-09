/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/07/09 11:15:04 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Request::Request() {}

Request::Request(char const *buffer, int read, int socket) {
	std::cout << "Constructor Request called" << std::endl;
	// std::string buff = buffer;

	// A UPDATE
	socket_fd = socket;
	save_buffer = buffer;
	dir = 0;
	// response_content = "";
	// port = "";
	// body = "";
	// content_type = "";
	// lenght = 0;
	
	if (read < BUFFER_SIZE)
		status = RD_TO_RESPOND;
	else
		status = READING;
	std::cout << "End of Constructor Request" << std::endl;
}

Request::Request(const Request & orig) : status(orig.status), save_buffer(orig.save_buffer) , socket_fd(orig.socket_fd), dir(0) {
	(void) orig;
}

Request::~Request() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */
/* ************************** Assignment Operator  ************************* */

Request &Request::operator=(Request const & rhs) {
	socket_fd = rhs.socket_fd;
	status = rhs.status;
	save_buffer = rhs.save_buffer;
	method = rhs.method;
	target = rhs.target;
	dir = rhs.dir;
	version = rhs.version;
	host = rhs.host;
	port = rhs.port;
	agent = rhs.agent;
	media = rhs.media;
	connection = rhs.connection;
	socket_s_addr = rhs.socket_s_addr;
	
	response = rhs.response;

	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

int Request::getStatus() const {
	return (status);
}

int Request::getSocket_fd() const {
	return (socket_fd);
}

in_addr_t Request::getSocket_s_addr() const {
	return (socket_s_addr);
}

std::string Request::getHost() const {
	return (host);
}

std::string Request::getConnection() const {
	return (connection);
}

std::string Request::getSave_buffer() const {
	return (save_buffer);
}

void	Request::setStatus(int status) {
	this->status = status;
}

void	Request::addSave_buffer(const char *buffer) {
	save_buffer += buffer;
}

/* ************************************************************************* */
/* ******************************** Parsing ******************************** */
/* ************************************************************************* */

void	Request::parse_request(in_addr_t s_addr) {
	
	socket_s_addr = s_addr;
	method = extract_line(save_buffer, ' ');
	target = extract_line(save_buffer, ' ');
	if (target.back() == '/')
		dir = 1;
	version = extract_line(save_buffer, '\r');
	host = extract_header(save_buffer);
	agent = extract_header(save_buffer);
	media = extract_header(save_buffer);
	connection = extract_elem("Connection:", "\r", save_buffer, "keep-alive");
	// std::cout << "connection : " << connection << std ::endl;
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

void	Request::send_response(int socket_fd) {
	
	response.setContent_length();
	std::string response_content = response.build_response();
	write(socket_fd, response_content.c_str(), response_content.size());
	status = SENT;
}

//  parse request from client and send back response 
int  Request::handle_request(int socket_fd, t_conf &conf, ErrorPages &error)
{
	int i = check_exist_method();
	std::cout << "i = " << i << std::endl;
	switch (i) {
		case UNKNOWN : {
			std::cout << "ERROR" << std::endl;
			error.fill_error(response, "405", conf);
			return (send_response(socket_fd), 1);
		}
		default : { // A separer GET == 0 POST == 1 DELETE == 2 ==> Pour l instant ne traite que le GET
			std::cout << "target " << target << std::endl;
			std::string index = look_for_location(conf); // si pas trouve index = ""
			if (index.empty())
			{
				if (conf.root_dir.empty()) {
					error.fill_error(response, "404", conf);
					return (send_response(socket_fd), 1);
				}
				else {
					if (conf.root_dir.back() == '/')
						target.replace(0, 1, conf.root_dir);
					else	
						target.insert(0, conf.root_dir);
				}
			}
			/* AVANT D AJOUTER LE PATH => check method ok */
			else {
				if (!check_allow_method(conf, index)) {
					error.fill_error(response, "405", conf);
					return (send_response(socket_fd), 1);
				}
				add_path(conf, index); // GET POST DELETE
			}
			std::cout << "target " << target << std::endl;
			// CGI
			return (build_response(socket_fd, conf, index, error), 1); // GET ONLY ICI
		}
	}
}

/* ************************************************************************* */
/* ******************************* Location ******************************** */
/* ************************************************************************* */

/*
Recherche pour la location de la plus precise a la moins precise dans le serveur
*/
// PENSER A TESTER SI / TOUJOURS AVEC CURL DANS REQUETE COMME POUR HTTP
std::string Request::look_if_location(std::string &target, t_conf & conf) {

	// std::map< std::string, std::map<std::string, std::string> >::iterator it;
	auto it = conf.location.find(target);
	if (it != conf.location.end())
		return (it->first);	
	std::string s = target;
	if (target.back() == '/')
		s.erase(s.length() - 1, 1); 
	else {
		size_t found = s.rfind('/');
		if (found == std::string::npos)
			return ("");
		s.erase(found + 1, s.length() - found + 1);
	}
	return (look_if_location(s, conf));
}

/*
Cherche si une location = correspondante a la target existe, sinon appelle look_if_location
*/
// A FAIRE : VOIR COMMENT GERER =/loc et =/loc/index.html
std::string Request::look_for_location(t_conf & conf) {
	
	std::string s = "=" + target;
	// std::map< std::string, std::map<std::string, std::string> >::iterator it;
	auto it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	return (look_if_location(target, conf)); //on recupere l index de la location :)
}

/* Supprime la location et la remplace par la root */
void	Request::add_path(t_conf & conf, std::string &index) {
	
	std::string root;
	
	if (conf.location[index].find("root") == conf.location[index].end())
		root = conf.root_dir;
	else
		root  = conf.location[index]["root"];
	if(index.back() == '/' && root.back() != '/')
		root += "/";
	if (index.front() == '=')
		index.erase(0, 1);
	target.replace(target.find(index), index.length(), root);
	// std::cout << "target : " << target << std::endl;
}

/* ************************************************************************* */
/* ********************************* Method ******************************** */
/* ************************************************************************* */

int	Request::check_exist_method() {
	
	std::string	method[3] = {"GET", "POST", "DELETE"};
	int i;
	
	for (i = 0; i < 3; i++) {
		if (this->method == method[i])
			break;
	}
	return (i);
}

bool	Request::check_allow_method(t_conf &conf, std::string &index) {
	
	if (conf.location[index].find("allow_methods") != conf.location[index].end()) {
		if (conf.location[index]["allow_methods"].find(this->method) == std::string::npos)
			return (false);
	}
	return (true);
}

/* ************************************************************************* */
/* ********************************** CGI ********************************** */
/* ************************************************************************* */


void	Request::exec_script(char const *pathname, char *const argv[], char *const envp[], t_conf & conf)
{
	int		pid;
	int		status;
	int		fd[2];
	int		rd;
	char	buff[1024];
	int		exit_status;
	std::string	output;
	
	pipe(fd);
	pid = fork();
	if (pid == -1)
		perror("fork");
	if (pid == 0)
	{
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
			if (execve(pathname, argv, envp) != 0)
			{
				perror("execve");
				exit (1);
			}
	}
	else
		waitpid(pid, &status, 0);
	close(fd[1]);
	if (WIFEXITED(status))
		exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		std::cout << "Rediriger vers page error 500\n";
	if (exit_status != 1)
	{
		while (1)
		{
			rd = read(fd[0], buff, sizeof(buff) - 1);
			if (rd < 1)
				break ;
			buff[rd] = '\0';
		}
		close(fd[0]);
		// std::cout << "BUFFER:\n" << buff << std::endl;
		response.setBody(buff);
		response.setStatus("200", " OK");
		response.setContent_type("text/html");
	}
	else
		throw ;
		
}

void	Request::set_env(t_conf & conf, char ***env)
{
	strcpy(*env[0], ("REQUEST_METHOD=" + method).c_str());
	strcpy(*env[1], ("QUERY_STRING=" + query_string).c_str());
	strcpy(*env[2], ("SCRIPT_NAME=" + script_name).c_str());
	strcpy(*env[3], ("SERVER_NAME=" + conf.server_name).c_str());
	strcpy(*env[4], ("SERVER_PORT=" + conf.ipv4_port[0]).c_str());
	strcpy(*env[5], ("REMOTE_ADDR=" + conf.host).c_str());
	strcpy(*env[6], ("HTTP_USER_AGENT=" + agent).c_str());
	*env[7] = NULL;
}

void    Request::handle_cgi(t_conf & conf)
{
	char	**env;
	char	**argv;

	std::cout << "target in CGI" << target << std::endl;
	std::string copy = target;
	script_name = copy.substr(0, copy.find('?'));
	std::string join = ("./" + script_name);
	char const *exec = join.c_str();
	copy.erase(0, copy.find('/') + 1);
	copy.erase(0, copy.find('/') + 1);

    argv = new char * [2];
    argv[0] = new char [50];
    argv[1] = NULL;
    strcpy(argv[0], "form_handler.cgi");
	env = new char* [8];
	for (int i = 0; i < 7; i++)
		env[i] = new char [1024];
	strcpy(env[0], ("REQUEST_METHOD=" + method).c_str());
	strcpy(env[1], ("QUERY_STRING=" + query_string).c_str());
	strcpy(env[2], ("SCRIPT_NAME=" + script_name).c_str());
	strcpy(env[3], ("SERVER_NAME=" + conf.server_name).c_str());
	strcpy(env[4], ("SERVER_PORT=" + conf.ipv4_port[0]).c_str());
	strcpy(env[5], ("REMOTE_ADDR=" + conf.host).c_str());
	strcpy(env[6], ("HTTP_USER_AGENT=" + agent).c_str());
	env[7] = NULL;
	// set_env(conf, &env);
	// char const *tmp = copy.substr(0, copy.rfind('?')).c_str();
	// char *exec = new char [sizeof(tmp)];
	// exec = strcpy(exec, tmp);
	// std::cout << "Exec = " << exec << std::endl;
	// char *const argv[] = {exec, NULL};
	// char *argv[2] = {"form_handler.cgi", NULL};
	copy.erase(0, copy.find('?') + 1);
	query_string = copy.substr(0, copy.npos);
	script_name.erase(0, script_name.find('/'));
	script_name.insert(0, "./cgi-bin");
	// std::cout << "script name = " << script_name << std::endl;
	exec_script(exec, argv, env, conf);
	// for (int i = 0; i < 7; i++)
	// 	std::cout << i << " " << env[i] << std::endl;
	for (int i = 0; i < 7; i++)
		delete env[i];
	delete env;
	delete argv[0];
	delete argv;
}

/* ************************************************************************* */
/* ********************************** GET ********************************** */
/* ************************************************************************* */

void	Request::build_response(int socket_fd, t_conf &conf, std::string &location, ErrorPages &error) {
	
	// std::cout << "\nLOCATION = " << location << std::endl;
	if (location.empty()) {
		// =====> Request is a directory (end with a "/")
		if (dir == 1)
			target_directory(conf, error);
		//  =====> Request isn't a directory
		else {
			if (!open_targetfile(target))
				error.fill_error(response, "404", conf);
		}
	}
	else {
		// =====> Request is a directory (end with a "/")
		if (dir == 1)
			target_directory(conf, location, error);
		//  =====> Request isn't a directory
		else if (location.find("/cgi-bin") != location.npos)
		{
			try
			{
				handle_cgi(conf);
			}
			catch(const std::exception& e)
			{
				error.fill_error(response, "404", conf);
			}
		}
		else {
			if (!open_targetfile(target))
				error.fill_error(response, "404", conf);
		}
	}
	send_response(socket_fd);
}

bool	Request::open_targetfile(std::string & target) {
	
	std::ifstream     file;
	
	file.open(target);
	if (file.is_open()) {
		response.setBody(file);
		response.setStatus("200", " OK");
		response.setContent_type("text/html"); // A REVOIR CAR DEPENDS DU TYPE DU FICHIER!!
		// a verifier text/html si css jss ou utilise avec curl ou http...
		return (true);
	}
	return (false);
}

/* Le champs index a la  priorite sur autoindex si les deux sont presents */
void	Request::target_directory(t_conf &conf, ErrorPages &error) {
	
	// for (std::vector<std::string>::iterator it = conf.files_vect.begin(); it != conf.files_vect.end(); it++) {
	for (auto it = conf.files_vect.begin(); it != conf.files_vect.end(); it++) {
		std::string tmp = target + *it;
		if (open_targetfile(tmp)) // =====> open le 1er index valide
			return ;
	}
	if (conf.autoindex == "on") // =====> Autoindex on
		return (build_index(), (void)0);
	error.fill_error(response, "404", conf);
}

/* Le champs index a la  priorite sur autoindex si les deux sont presents */
void	Request::target_directory(t_conf &conf, std::string &location, ErrorPages &error) {
	
	// std::map<std::string, std::string>::iterator it = conf.location[location].find("index");
	auto it = conf.location[location].find("index");
	if (it != conf.location[location].end()) { // =====> Index in location present
		std::vector<std::string> index;
		strtovect(it->second, index, " ");
		// for (std::vector<std::string>::iterator jt = index.begin(); jt != index.end(); jt++) {
		for (auto jt = index.begin(); jt != index.end(); jt++) {
			std::string tmp = target + *jt;
			if (open_targetfile(tmp)) // =====> open le 1er index valide
				return ;
		}
	}
	it = conf.location[location].find("autoindex"); // =====> Index in location absent or index page doesn't exist
	if (it == conf.location[location].end() || it->second != "on") // =====> Autoindex off or absent
		return (error.fill_error(response, "404", conf), (void)0);
	else // =====> Autoindex on
		build_index();
}

/*  Quand Autoindex est on : Cree la page html de l'index */
void	Request::build_index() {
	
	DIR *tmp = opendir(target.data());
	dirent *directory;
	
	/* A TESTER : RENTRER DANS UN DOSSIER NULL */
	response.setBody("<!DOCTYPE html>\n<html>\n<head>\n<title>Index</title>\n</head>\n<body>\n<h3>Index</h3>\n");
	while (1) {
		directory = readdir(tmp);
		if (!directory)
			break; // =====> readdir a fini de lister
		std::string link = directory->d_name;
		if (link == "." || link == "..")
			continue;
		if (static_cast<int>(directory->d_type) == 4) // =====> Si le lien est un dir = 4 si html =8
			link += "/";
		response.setBody("<p><a href=\"");
		response.setBody(link);
		response.setBody("\">");
		response.setBody(directory->d_name);
		response.setBody("</a></p>\n");
	}
	closedir(tmp);
	response.setBody("</body>");
	response.setBody("</html>");
	response.setStatus("200", " OK");
	response.setContent_type("text/html"); // Type ok : l'index auto genere est html
}

/* ************************************************************************* */
/* ********************************* Utils ********************************* */
/* ************************************************************************* */

std::string Request::extract_line(std::string & buff, char delim) const
{
	int end = buff.find(delim);
	std::string tmp (buff.substr(0, end));
	if (delim == '\r')
	  end += 1;
	buff.erase(0, end + 1);
	return(tmp);
}

std::string Request::extract_header(std::string & buff) const
{
	int end = buff.find(' ');
	buff.erase(0, end + 1);
	end = buff.find('\r');
	std::string tmp (buff.substr(0, end));
	buff.erase(0, end + 2);
	return (tmp);
}

std::string Request::extract_elem(std::string const & elem, std::string const & delim, std::string & buff, std::string const & nofound) const {
	
	int begin = buff.find(elem);
	if (begin == -1)
		return (nofound);
	int end = buff.find(delim);
	std::string tmp (buff.substr(begin, end + 1));
	return (extract_header(tmp));
}
