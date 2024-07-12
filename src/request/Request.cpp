/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/07/12 18:04:21 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"
#include <sys/stat.h>
/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Request::Request() {}

Request::Request(char const *buffer, int read, int socket, Server *src_server, Media *src_auth_media) {
	// A UPDATE
	socket_fd = socket;
	server = src_server;
	auth_media = src_auth_media;
	response.setAuthmedia(src_auth_media);
	if (buffer)
		save_buffer = buffer;
	i_conf = -1;
	if (read < BUFFER_SIZE)
		status = RD_TO_RESPOND;
	else
		status = READING;
}

Request::Request(const Request & orig) : socket_fd(orig.socket_fd), status(orig.status), save_buffer(orig.save_buffer) {
	*this = orig;
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
	version = rhs.version;
	host = rhs.host;
	port = rhs.port;
	agent = rhs.agent;
	media = rhs.media;
	connection = rhs.connection;
	socket_s_addr = rhs.socket_s_addr;
	socket_ip = rhs.socket_ip;
	server = rhs.server;
	auth_media = rhs.auth_media;
	i_conf = rhs.i_conf;
	_query_string = rhs._query_string;
	_script_name = rhs._script_name;
	
	response = rhs.response;

	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

// Response &Request::getsetResponse() {
// 	return (response);
// }

int 		Request::getIconf() const {
	return (i_conf);
}

int Request::getStatus() const {
	return (status);
}

int Request::getSocket_fd() const {
	return (socket_fd);
}

in_addr_t Request::getSocket_s_addr() const {
	return (socket_s_addr);
}

std::string Request::getSocket_ip() const {
	return (socket_ip);
}

std::string Request::getHost() const {
	return (host);
}

std::string Request::getPort() const {
	return (port);
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
/* ******************************* Exception ******************************* */
/* ************************************************************************* */



/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

void	Request::send_response(int socket_fd) {
	
	response.setContent_length();
	std::string response_content = response.build_response();
	write(socket_fd, response_content.c_str(), response_content.size());
	std::cout << "********** RESPONSE CONTENT **********" << std::endl;
	std::cout << response_content << std::endl;
	std::cout << "******************************" << std::endl;
	status = SENT;
}

//  parse request from client and send back response 
int  Request::handle_request(int socket_fd, t_conf &conf, ErrorPages &error) // return necessaire?
{
	if (!check_request(socket_fd, conf, error))
		return (1);
	int i = check_exist_method();
	// std::cout << "i = " << i << std::endl;
	switch (i) {
		case UNKNOWN : {
			error.fill_error(response, "405", conf);
			return (send_response(socket_fd), 1);
		}
		default : { // A separer GET == 0 POST == 1 DELETE == 2 ==> Pour l instant ne traite que le GET
			// std::cout << "target " << target << std::endl;
			std::string index = look_for_location(target, conf); // si pas trouve index = ""
			if (index.empty())
			{
				if (conf.root_dir.empty()) {
					error.fill_error(response, "404", conf);
					return (send_response(socket_fd), 1);
				}
				else {
					if (strback(conf.root_dir) == '/')
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
			cgi_parse_target();
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

	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(target);
	if (it != conf.location.end())
		return (it->first);	
	std::string s = target;
	if (strback(target) == '/')
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
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	return (look_if_location(target, conf)); //on recupere l index de la location :)
}

/*
Cherche si une location = correspondante a la target existe, sinon appelle look_if_location
*/
// A FAIRE : VOIR COMMENT GERER =/loc et =/loc/index.html
std::string Request::look_for_location(std::string &uri, t_conf & conf) {
	
	std::string s = "=" + uri;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	return (look_if_location(uri, conf)); //on recupere l index de la location :)
}

/* Supprime la location et la remplace par la root dans target */
void	Request::add_path(t_conf & conf, std::string &index) {
	
	std::string root;
	
	if (conf.location[index].find("root") == conf.location[index].end())
		root = conf.root_dir;
	else
		root  = conf.location[index]["root"];
	if(strback(index) == '/' && strback(root) != '/')
		root += "/";
	if (index[0] == '=')
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

char**	Request::set_env(t_conf & conf)
{
	char	**env;

	std::string copy = target;
	copy.erase(0, copy.find('/') + 1);
	copy.erase(0, copy.find('/') + 1);
	env = new char* [8];
	for (int i = 0; i < 7; i++)
		env[i] = new char [1024];
	strcpy(env[0], ("REQUEST_METHOD=" + method).c_str());
	strcpy(env[1], ("QUERY_STRING=" + _query_string).c_str());
	strcpy(env[2], ("SCRIPT_NAME=" + _script_name).c_str());
	strcpy(env[3], ("SERVER_NAME=" + conf.server_name).c_str());
	strcpy(env[4], ("SERVER_PORT=" + conf.ipv4_port[0]).c_str());
	strcpy(env[5], ("REMOTE_ADDR=" + conf.host).c_str());
	strcpy(env[6], ("HTTP_USER_AGENT=" + agent).c_str());
	env[7] = NULL;
	return (env);
}

int	Request::exec_script(char const *pathname, char *const argv[], char *const envp[])
{
	int		pid;
	int		status;
	int		fd[2];
	int		rd;
	char	buff[1024];
	int		exit_status = 0;
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
			for (int i = 0; i < 7; i++)
				delete [] envp[i];
			delete [] envp;
			delete [] argv;
			delete [] server->fds;
			perror("execve");
			exit (1);
		}
	}
	waitpid(pid, &status, 0);
	close(fd[1]);
	if (WIFEXITED(status))
		exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		throw RequestException ("500");
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
		response.setContent_type("html");
		response.setBody(buff);
		response.setStatus("200", " OK");
	}
	else
	{
		close(fd[0]);
		for (int i = 0; i < 7; i++)
			delete [] envp[i];
		delete [] envp;
		delete [] argv;
		throw RequestException ("404");
	}
	return (0);
}

void	Request::check_extension(t_conf & conf, std::string target)
{	
	std::vector<std::string>	cgi_ext;
	std::string					ext;
	
	for (std::map<std::string, std::map<std::string, std::string> >::iterator it = conf.location.begin(); it != conf.location.end(); it++)
	{
		for (std::map<std::string, std::string>::iterator its = (*it).second.begin(); its != (*it).second.end(); its++)
		{
			if ((*its).first.find("cgi_extension") != (*its).second.npos)
				strtovect((*its).second, cgi_ext, " ");
		}
	}
	target = target.erase(0, target.rfind('.') + 1);
	ext = target.substr(0, target.find('?'));
	if (std::count(cgi_ext.begin(), cgi_ext.end(), ext) == 0)
		throw RequestException ("501");
}

bool	Request::is_accessible(char const *target)
{
	struct stat path_stat;
	stat(target, &path_stat);
	if (access(target, X_OK) == -1)
	{
		std::cout << target << std::endl;
		return false;
	}
	return S_ISREG(path_stat.st_mode);
}

char const *	define_ext(std::string target)
{
	target.erase(0, target.find('.') + 1);
	if (target.substr(0, target.find('?')) == "py")
		return ("/usr/bin/python3");
	else 
		return ("/usr/bin/php");
}

void    		Request::handle_cgi(t_conf & conf)
{
	char		**env;
	char		**argv;

	std::string join = ("./" + _script_name);
	char const *pathname = join.c_str();
	if (!is_accessible(pathname))
		throw RequestException ("404");
	check_extension(conf, target);
	char const * interpreter = define_ext(target);
    argv = new char * [3];
	argv[0] = (char *) interpreter;
    argv[1] = (char *) pathname;
    argv[2] = NULL;
	env = set_env(conf);
	exec_script(argv[0], argv, env);
	for (int i = 0; i < 7; i++)
		delete [] env[i];
	delete [] env;
	delete [] argv;
}

/* ************************************************************************* */
/* ******************************** Cookies ******************************** */
/* ************************************************************************* */

//pour acceder a request, passer le ptr sur server

void	Request::setTimestamp(std::ofstream	& data)
{
	// ecrire ici: si la difference de temps entre timestamp actuel et celui de data_user trop important, refresh info
	std::time_t result = std::time(NULL);
	data << "TimeStamp=" << std::asctime(std::localtime(&result)) << std::endl;
}

void	Request::create_data_file(void)
{
	std::ofstream	data("Data_user", std::ofstream::out);
	std::string		copy = body;
	
	for (int i = 0; i < 2; i++)
	{
		data << copy.substr(0, copy.find('&')) << std::endl;
		copy.erase(0, copy.find('&') + 1);
	}
	setTimestamp(data);
}

void	Request::setSession(void)
{
	create_data_file();
}

// void	Response::setCookies(std::string fname, std::string lname)
// {
	// std::string str[2];
	// (void) lname;
	// (void) fname;
	// _cookie = "Set-Cookie: path=/form.html; HttpOnly";
// }

void	Request::handle_cookies(void)
{
	if (body.find("rememberMe=on") != body.npos)
	{
		std::cout << "OPEN SESSION\n";
		setSession();
	}
	
}

/* ************************************************************************* */
/* ********************************** GET ********************************** */
/* ************************************************************************* */

// A FAIRE : 
//		setContenttype() en fonction du language du fichier  
//		implementer directive return dans location ou server  ===> Gerer dans parsing conf les cas d erreur de return si on a pas format code + " " + strig
//		voir si on arrive a envoyer une image  

void	Request::build_response(int socket_fd, t_conf &conf, std::string &location, ErrorPages &error) {
	
	// std::cout << "\nLOCATION = " << location << std::endl;
	// std::cout << "TARGET = " << target << std::endl;
	// std::cout << "DIR = " << dir << std::endl;
	if (location.empty()) {
		// 	=====> Server has a return
		if (!conf.ret.empty())
			redirection(conf.ret, error, location, conf); // A FAIRE : GERER CAS D ERREUR >> TESTER PARSING CONF RETURN
		// 	=====> Request is a directory (end with a "/")
		else if (is_dir(target))
			target_directory(conf, error);
		//  =====> Request isn't a directory
		else {
			if (!open_targetfile(target, error, conf))
				error.fill_error(response, "404", conf);
		}
	}
	else {
		// 	=====> Location has a return
		std::map<std::string, std::string>::iterator it = conf.location[location].find("return");
		if (it != conf.location[location].end())
			redirection(it->second, error, location, conf); // A FAIRE : GERER CAS D ERREUR >> TESTER PARSING CONF RETURN
		// 	=====> Request is a directory (end with a "/")
		
		else if (is_dir(target))
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
				error.fill_error(response, e.what(), conf);
			}
		}
		else
			if (!open_targetfile(target, error, conf))
				error.fill_error(response, "404", conf);
	}
	handle_cookies();
	send_response(socket_fd);
}

bool	Request::open_targetfile(std::string & target, ErrorPages & error, t_conf &conf) {
	
	std::ifstream     file;
	
	file.open(target.data());
	if (file.is_open()) {
		response.setBody(file);
		response.setStatus("200", " OK");
		std::string	type = extract_extension(target);
		if (!response.setContent_type(type)) {
			response.reinitBody();
			error.fill_error(response, "405", conf);
		}
		return (true);
	}
	return (false);
}

bool	Request::is_dir(std::string const &path) {
	struct stat buf;
	if (stat(path.data(), &buf) == -1)
		std::cout << "A IMPLEMENTER CAS ERREUR" << std::endl;
	if (S_ISDIR(buf.st_mode))
		return (true);
	return (false);
}

/* Le champs index a la  priorite sur autoindex si les deux sont presents */
void	Request::target_directory(t_conf &conf, ErrorPages &error) {
	
	for (std::vector<std::string>::iterator it = conf.files_vect.begin(); it != conf.files_vect.end(); it++) {
		std::string tmp = target + *it;
		if (open_targetfile(tmp, error, conf)) // =====> open le 1er index valide
			return (std::cout << "return" << std::endl, (void)0);
	}
	if (conf.autoindex == "on") // =====> Autoindex on
		return (build_index(), (void)0);
	error.fill_error(response, "404", conf);
}

/* Le champs index a la  priorite sur autoindex si les deux sont presents */
void	Request::target_directory(t_conf &conf, std::string &location, ErrorPages &error) {
	
	std::map<std::string, std::string>::iterator it = conf.location[location].find("index");
	if (it != conf.location[location].end()) { // =====> Index in location present
		std::vector<std::string> index;
		strtovect(it->second, index, " ");
		for (std::vector<std::string>::iterator jt = index.begin(); jt != index.end(); jt++) {
			std::string tmp = target + *jt;
			if (open_targetfile(tmp, error, conf)) // =====> open le 1er index valide
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
	response.setContent_type("html"); // Type ok : l'index auto genere est html
}

bool	Request::is_loop(std::string &redir, std::string const &location, t_conf &conf) {
	
	std::string tmp = look_for_location(redir, conf);
	std::cout << "redir = " << redir << std::endl; 
	std::cout << "next location = " << tmp << std::endl; 
	if (tmp == location)
		return (true);
	return (false);
}

void	Request::redirection(std::string const &ret, ErrorPages &error, std::string const &location, t_conf &conf) {
	
	std::string code = ret.substr(0, ret.find(' '));
	std::string redir = ret.substr(ret.find(' ') + 1);
	redir = "/" + redir;
	if (is_loop(redir, location, conf))
		return (error.fill_error(response, "508", conf), (void)0);
	redir = "http://" + host + ":" + port + redir;
	error.fill_redir(response, code, redir);
}

/* ************************************************************************* */
/* ********************************** POST ********************************* */
/* ************************************************************************* */

/* ************************************************************************* */
/* ******************************** Parsing ******************************** */
/* ************************************************************************* */

bool	Request::media_request_allowed() {
	
	for (std::map<std::string, std::vector<std::string> >::iterator it=media.begin() ; it != media.end(); it++) {
		for (std::vector<std::string>::iterator jt = it->second.begin(); jt != it->second.end(); jt++)
			if (auth_media->is_allow(it->first, *jt))
				return (true);
	}
	return (false);
}

bool	Request::check_request(int socket_fd, t_conf &conf, ErrorPages &error) {
	
	if (miss_length && !body.empty()) {
		error.fill_error(response, "411", conf);
		return (send_response(socket_fd), false);
	}
	if (/* method.empty() || version.empty() || target.empty() || host.empty() || port.empty() ||  */content_length < 0 || static_cast<size_t>(content_length) != body.length()) {
		error.fill_error(response, "400", conf);
		return (send_response(socket_fd), false);
	}
	if (version != "HTTP/1.1") {
		error.fill_error(response, "405", conf);
		return (send_response(socket_fd), false);
	}
	if (content_length > conf.limit_body_size) {
		error.fill_error(response, "413", conf);
		return (send_response(socket_fd), false);
	}
	if (!media_request_allowed()) {
		error.fill_error(response, "415", conf);
		return (send_response(socket_fd), false);
	}
	return (true);
}

void	Request::recover_ip_socket() {
	u_int32_t n = ntohl(socket_s_addr);
	std::stringstream iss;
	for (int i = 24; i >= 0; i -= 8) {
		int tmp = (n >> i) & 0xFF;
		iss << tmp;
		if (i > 0)
			iss << '.';
	}
	socket_ip = iss.str();
	std::cout << socket_ip << std::endl;
}

void	Request::cgi_parse_target() {
    
    size_t found = target.find('?');

	if (found != std::string::npos) {
		_script_name = target.substr(0, found);
		if (found < target.length() - 1)
			_query_string = target.substr(found + 1);
	}
	else if (method == "POST") {
		_script_name = target;
		_query_string = body;
	}
	else
		_script_name = target;
}

void	Request::parse_host() {
	std::istringstream	iss(host);
	
	std::getline(iss, host, ':');
	str_tolower(host);
	std::getline(iss, port);
}

void	Request::parse_media(std::string &s) {

std::string tmp;
size_t begin, end;

while (1) {
	tmp = s;
	begin = tmp.find(";q=");
	if (begin == std::string::npos)
		break;
	end = tmp.find(",", begin);
	s = tmp.erase(begin, end - begin);
}
strtomap(s, media, ",", "/");
// for (std::map<std::string, std::vector<std::string> >::iterator it=media.begin() ; it != media.end(); it++) {
// 	std::cout << "MEDIA[" << it->first << "] = ";
// 	for (std::vector<std::string>::iterator jt = it->second.begin(); jt != it->second.end(); jt++)
// 		std::cout << *jt << " ";
// 	std::cout << std::endl;
// 	}
}

bool	Request::parse_first_line(in_addr_t s_addr, ErrorPages &error) {
	
	socket_s_addr = s_addr;
	recover_ip_socket();
	/* Parse first line */
	method = extract_line(save_buffer, ' ');
	target = extract_line(save_buffer, ' ');
	version = extract_line(save_buffer, '\r');
	host = extract_elem("Host:", "\r", save_buffer, "");
	parse_host();
	if (method.empty() || version.empty() || target.empty() || host.empty() || port.empty()) {
		error.fill_error(response, "400");
		return (send_response(socket_fd), false);
	}
	return (true);
}

void	Request::parse_request() {
	std::string tmp;
	
	/* Extract Headers */
	agent = extract_elem("User-Agent:", "\r", save_buffer, "");
	tmp = extract_elem("Accept:", "\r", save_buffer, "");
	if (!tmp.empty())
		parse_media(tmp);
	connection = extract_elem("Connection:", "\r", save_buffer, "keep-alive");
	tmp = extract_elem("Content-Length:", "\r", save_buffer, "0");
	if (tmp.empty())
		miss_length = true;
	else {
		miss_length = false;
		try { content_length = ft_stoi(tmp); }
		catch (std::exception & e) { content_length = -1; }
	}
	content_type = extract_elem("Content-Type:", "\r", save_buffer, "");
	body = extract_body(save_buffer);
	// std::cout << std::endl;
	// std::cout << "host : " << host << std ::endl;
	// std::cout << "port : " << port << std ::endl;
	// std::cout << "method : " << method << std ::endl;
	// std::cout << "target : " << target << std ::endl;
	// std::cout << "version : " << version << std ::endl;
	// std::cout << "content_length : " << content_length << std ::endl;
	// std::cout << "connection : " << connection << std ::endl;
	// std::cout << "body : " << body << std ::endl;
	// std::cout << "*******************************" << std ::endl;
	// std::cout << "save_buffer : " << save_buffer << std ::endl;
	// std::cout << "*******************************" << std ::endl;
	// std::cout << std::endl;
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
	
	int begin, end = 0, sep_body;
	sep_body = buff.find("\n\r\n");
	begin = buff.find(elem);
	if (begin == -1 || (sep_body > -1 && begin > sep_body))
		return (nofound);
	end = buff.find(delim, begin);
	std::string tmp (buff.substr(begin, end + 1));
	return (extract_header(tmp));
}

std::string Request::extract_body(std::string & buff) {
	
	int begin = buff.find("\n\r\n");
	if (begin == -1 || static_cast<size_t>(begin + 3) >= buff.length())
		return ("");
	std::string tmp (buff.substr(begin + 3, buff.length() - begin + 3));
	return (tmp);
}

std::string Request::extract_extension(std::string const & s) {
	
	int found = s.find_last_of('.');
	if (found == -1 || found == static_cast<int>(s.length() - 1))
		std::cout << "A IMPLEMENTER" << std::endl;
	return (s.substr(found + 1));
}

/* ************************************************************************* */
/* ********************************* CLOSE ********************************* */
/* ************************************************************************* */	

void	Request::handle_pending_requests(ErrorPages & error, int & socket) {
	
	response.reinitBody();
	if (i_conf > -1)
		error.fill_error(response, "503", server->conf[i_conf]);
	else
		error.fill_error(response, "503");
	send_response(socket);
}
