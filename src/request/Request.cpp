/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/09/02 10:39:51 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"
#include <sys/stat.h>
/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Request::Request() {}

Request::Request(char const *buffer, int read, int socket, Server *src_server, ErrorPages *src_error, Media *src_auth_media/* , int id */) {

	time_t	now;
	time(&now);
	t_creation =  localtime(&now);
	socket_fd = socket;
	server = src_server;
	error = src_error;
	auth_media = src_auth_media;
	response.setAuthmedia(src_auth_media);
	if (buffer)
		save_buffer.append(buffer, read);
	i_conf = -1;
	status = NEW;
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
	socket_s_addr = rhs.socket_s_addr;
	socket_ip = rhs.socket_ip;
	status = rhs.status;
	save_buffer = rhs.save_buffer;
	method = rhs.method;
	uri = rhs.uri;
	version = rhs.version;
	_query_string = rhs._query_string;
	_target = rhs._target;
	host = rhs.host;
	port = rhs.port;
	agent = rhs.agent;
	media = rhs.media;
	connection = rhs.connection;
	content_type = rhs.content_type;
	transfer_encoding = rhs.transfer_encoding;
	content_length = rhs.content_length;
	miss_length = rhs.miss_length;
	body = rhs.body;
	response = rhs.response;
	server = rhs.server;
	error = rhs.error;
	auth_media = rhs.auth_media;
	i_conf = rhs.i_conf;
	t_creation = rhs.t_creation;
	boundary = rhs.boundary;
	body_deque = rhs.body_deque;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

int 		Request::getIconf() const {
	return (i_conf);
}

int Request::getStatus() const {
	return (status);
}

tm 		*Request::getT_creation() const {
	return (t_creation);
}

void 		Request::setT_creation(time_t &now) {
	t_creation = localtime(&now);
}

int Request::getSocket_fd() const {
	return (socket_fd);
}

size_t 		Request::getSave_buffer_length() const{
	return (save_buffer.length());
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

std::string Request::getTransfer_encoding() const {
	return (transfer_encoding);
}

std::string Request::getContentType() const {
	return (content_type);
}

std::string Request::getConnection() const {
	return (connection);
}

std::string Request::getSave_buffer() const {
	return (save_buffer);
}

std::string Request::getBody() const{
	return (body);
} //A EFFACER


void	Request::setStatus(int status) {
	this->status = status;
}

void	Request::addSave_buffer(/* const */ char *buffer, int end) {
	
	save_buffer.append(buffer, end);
}

// void	Request::addSave_buffer(const char *buffer) {
// 	save_buffer += buffer;
// }

/* ************************************************************************* */
/* ******************************* Exception ******************************* */
/* ************************************************************************* */



/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

void	Request::send_response(int socket_fd) {
	
	response.setContent_length();
	std::string response_content = response.build_response();
	// int fd = write(socket_fd, response_content.c_str(), response_content.size());
	int fd = send(socket_fd, response_content.c_str(), response_content.size(), MSG_NOSIGNAL);
	if (fd == -1) {
		status = CLOSE;
		throw(ServerException("Fail to write"));
	}
}

//  parse request from client and send back response 
void  Request::handle_request(/* int socket_fd,  */t_conf &conf, ErrorPages &error) // return necessaire?
{
	// std::cout << "Request::handle_request 0 \n";
	check_request(conf, error);
	// std::cout << "Request::handle_request 1 \n";
	int i = check_exist_method();
	// std::cout << "Request::handle_request 2 \n";
	switch (i) {
		case UNKNOWN : {
			fill_significant_error("405", error, conf);
			return ;
		}
		case DELETE : {
			std::string index = find_location(conf, error);
			if (conf.location[index].find("return") != conf.location[index].end()) {
				fill_significant_error("405", error, conf);
			}
			delete_action(_target.data(), socket_fd, conf,  error);
			response.setStatus("204", error);
			setStatus(RD_TO_SEND);
			return ;
		}
		default : {
			// std::cout << "Request::handle_request 3 \n";
			std::string index = find_location(conf, error);
			// std::cout << "Request::handle_request 4 \n";
			return (build_response(conf, index, error), (void)0);
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
std::string Request::look_if_location(std::string &uri, t_conf & conf) {

	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(uri);
	if (it != conf.location.end())
		return (it->first);	
	std::string s = uri;
	if (strback(uri) == '/')
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
Cherche si une location = correspondante a la uri existe, sinon appelle look_if_location
*/
// A FAIRE : VOIR COMMENT GERER =/loc et =/loc/index.html
std::string Request::look_for_location(t_conf & conf) {
	
	std::string s = "=" + uri;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	return (look_if_location(uri, conf)); //on recupere l index de la location :)
}

/*
Cherche si une location = correspondante a la uri existe, sinon appelle look_if_location
*/
// A FAIRE : VOIR COMMENT GERER =/loc et =/loc/index.html
std::string Request::look_for_location(std::string &url, t_conf & conf) {
	
	std::string s = "=" + url;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	return (look_if_location(url, conf)); //on recupere l index de la location :)
}

/* Supprime la location et la remplace par la root dans uri */
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
	uri.replace(uri.find(index), index.length(), root);
}

std::string	Request::find_location(t_conf &conf, ErrorPages &error) {
	std::string index = look_for_location(uri, conf); // si pas trouve index = ""
	if (index.empty())
	{
		if (conf.root_dir.empty()) {
			fill_error("404", error, conf);
			setStatus(RD_TO_SEND);
			throw (RequestException(""));
		}
		else {
			if (strback(conf.root_dir) == '/')
				uri.replace(0, 1, conf.root_dir);
			else	
				uri.insert(0, conf.root_dir);
		}
	}
	/* AVANT D AJOUTER LE PATH => check method ok */
	else {
		if (!check_allow_method(conf, index)) {
			fill_significant_error("405", error, conf);
		}
		add_path(conf, index); // GET POST DELETE
	}
	cgi_parse_uri();
	return (index);
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

	std::string copy = uri;
	copy.erase(0, copy.find('/') + 1);
	copy.erase(0, copy.find('/') + 1);
	env = new char* [8];
	for (int i = 0; i < 7; i++)
		env[i] = new char [1024];
	strcpy(env[0], ("REQUEST_METHOD=" + method).c_str());
	strcpy(env[1], ("QUERY_STRING=" + _query_string).c_str());
	strcpy(env[2], ("SCRIPT_NAME=" + _target).c_str());
	strcpy(env[3], ("SERVER_NAME=" + conf.server_name).c_str());
	strcpy(env[4], ("SERVER_PORT=" + conf.ipv4_port[0]).c_str());
	strcpy(env[5], ("REMOTE_ADDR=" + conf.host).c_str());
	strcpy(env[6], ("HTTP_USER_AGENT=" + agent).c_str());
	env[7] = NULL;
	return (env);
}

void	Request::get_output(/*const */char *buff, t_conf &conf)
{
	int	flag = 0;
	std::string str = buff;
	std::cout << "str = " << str << std::endl;
	if (str == "1")
		fill_significant_error("500", *error, conf);
	while (1)
	{
		size_t pos_cookie = str.find(("Set-Cookie:"));
		size_t pos_loc = str.find(("Location: http"));
		if (pos_cookie != str.npos)
		{
			std::string extract = str.substr(pos_cookie, str.find('\n') - pos_cookie);
			response.setCookie(extract);
			str.erase(pos_cookie, str.find('\n') - pos_cookie + 1);
			continue ;
		}
		else if (pos_loc != str.npos)
		{
			size_t pos_http = str.find(("http"));
			str.erase(pos_loc, pos_http - pos_loc);
			pos_http = str.find(("http"));
			std::string extract = str.substr(pos_http, str.find('\n') - pos_http);
			response.setLocation(extract);
			str.erase(pos_http, str.find('\n') - pos_http + 1);
			response.setStatus("307", " Temporary Redirect");
			flag = 1;
		}
		else
		{
			response.setBody(str);
			break ;
		}
	}
	if (flag == 0)
		response.setStatus("200", " OK");
	response.setContent_type("html");
}

int	Request::exec_script(char const *pathname, char *const argv[], char *const envp[], t_conf &conf)
{
	int			script_pid;
	int			timer_pid;
	int			pid = 0;
	int			status;
	int			fd[2];
	int			rd;
	char		buff[1024];
	int			exit_status = 0;
	bool		script_got_killed = false;

	if (pipe(fd) == -1)
		fill_significant_error("500", *error, conf);
	script_pid = fork();
	if (script_pid == -1)
		fill_significant_error("500", *error, conf);
	if (script_pid == 0)
	{
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		server->close_child_sockets();
		std::cerr << "pathname" << std::endl << pathname << std::endl;
		if (execve(pathname, argv, envp) != 0)
		{
			deleteArgs(argv, envp);
			delete [] server->fds;
			server->del_all();
			exit (1);
		}
	}
	timer_pid = fork();
	if (timer_pid == -1)
		fill_significant_error("500", *error, conf);
	if (timer_pid == 0)
	{
		close(fd[0]);
		close(fd[1]);
		server->close_child_sockets();
		sleep(5);
		deleteArgs(argv, envp);
		delete [] server->fds;
		server->del_all();
		exit(0);
	}
	while (1)
	{
		pid = waitpid(WAIT_ANY, &status, 0);
		if (pid == script_pid || pid == timer_pid)
			break ;
	}
	if (pid == timer_pid)
	{
		kill(script_pid, SIGKILL);
		script_got_killed = true;
	}
	else 
		kill(timer_pid, SIGKILL);
	close(fd[1]);
	if (WIFEXITED(status))
		exit_status = WEXITSTATUS(status);
	if (script_got_killed == true)
	{
		close(fd[0]);
		deleteArgs(argv, envp);
		fill_significant_error("500", *error, conf);
		return (0);
	}
	if (exit_status == 0)
	{
		while (1)
		{
			rd = read(fd[0], buff, sizeof(buff) - 1);
			if (rd < 1)
				break ;
			buff[rd] = '\0';
		}
		close(fd[0]);
		get_output(buff, conf);
	}
	else
	{
		close(fd[0]);
		deleteArgs(argv, envp);
		fill_significant_error("500", *error, conf);
	}
	return (0);
}

void	Request::check_extension(t_conf & conf, std::string pathname, std::string & index_loc)
{	
	std::vector<std::string>	cgi_ext;
	std::string					ext;
	
	for (std::map<std::string, std::string>::iterator it = conf.location[index_loc].begin(); it != conf.location[index_loc].end(); it++)
		strtovect((*it).second, cgi_ext, " ");
	pathname = pathname.erase(0, pathname.rfind('.') + 1);
	ext = pathname.substr(0, pathname.find('?'));
	if (std::count(cgi_ext.begin(), cgi_ext.end(), ext) == 0)
		fill_significant_error("501", *error, conf);
}

bool	Request::is_accessible(char const *pathname)
{
	struct stat path_stat;
	stat(pathname, &path_stat);
	// SECU stat if == -1 ?
	if (access(pathname, X_OK) == -1)
		return false;
	return S_ISREG(path_stat.st_mode);
}

char const *	define_ext(std::string pathname)
{
	std::map<std::string, char const *> extension;
	setExtensions(extension);
	std::string ext = pathname.erase(0, pathname.rfind('.') + 1);
	return (extension[ext]);
}

void    		Request::handle_cgi(t_conf & conf, std::string & index_loc)
{
	char			**env;
	char			**argv;

	str = ("./" + _target);
	char const *pathname = str.c_str();
	if (_target.empty())
		fill_significant_error("400", *error, conf);
	struct stat path_stat;
	if (stat(pathname, &path_stat) == -1)
		fill_significant_error("400", *error, conf);
	std::ifstream	file(pathname, std::ifstream::in);
	if (!is_accessible(pathname) || is_empty(file))
		fill_significant_error("500", *error, conf);
	file.close();
	check_extension(conf, pathname, index_loc);
	char const * interpreter = define_ext(pathname);
	env = set_env(conf);
    argv = new char * [3];
	argv[0] = (char *) interpreter;
    argv[1] = (char *) pathname;
    argv[2] = NULL;
	exec_script(argv[0], argv, env, conf);
	deleteArgs(argv, env);
}

bool is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

/* ************************************************************************* */
/* ******************************** DELETE ********************************* */
/* ************************************************************************* */

/*
Que la requete delete concerne un fichier / dossier existant ou inexistant le code renvoye sera le meme : 204
Le serveur l efface dans le premier cas et ne fait rien dans le deuxieme
*/		
void	Request::delete_action(const char *target, int socket_fd, t_conf &conf, ErrorPages &error) {
	struct stat buf;
	
	if (stat(target, &buf) == -1)
		return ;
	if (S_ISDIR(buf.st_mode)) {
		DIR *tmp = opendir(target);
		if (!tmp) {
			fill_significant_error("403", error, conf);
		}
		dirent	*directory;
		while (1) {
			directory = readdir(tmp);
			if (!directory) { // OK NO LEAKS MEMMORY + FD
				if (remove(target) == -1) {
					fill_significant_error("500", error, conf);
				}
				break;
			}
			if (!strcmp(directory->d_name, ".") || !strcmp(directory->d_name, ".."))
				continue;
			std::string tmp = target;
			tmp += "/";
			tmp += directory->d_name;
			delete_action(tmp.data(), socket_fd, conf, error);
		}
		closedir(tmp);
	}
	else {
		if (remove(target) == -1) {
			fill_significant_error("500", error, conf);
		}
	}
}

/* ************************************************************************* */
/* ********************************** GET ********************************** */
/* ************************************************************************* */

// A FAIRE : 
//		setContenttype() en fonction du language du fichier  
//		implementer directive return dans location ou server  ===> Gerer dans parsing conf les cas d erreur de return si on a pas format code + " " + strig
//		voir si on arrive a envoyer une image  

void	Request::build_response(/* int socket_fd, */ t_conf &conf, std::string &location, ErrorPages &error) {
	
	// std::cout << "Request::build_response 0 \n";
	if (location.empty()) {
		// 	=====> Server has a return
		if (!conf.ret.empty())
			redirection(conf.ret, error, location, conf); // A FAIRE : GERER CAS D ERREUR >> TESTER PARSING CONF RETURN
		// 	=====> Request is a directory (end with a "/")
		else if (is_dir(_target))
			uri_directory(conf, error);
		//  =====> Request isn't a directory
		else {
			if (!open_targetfile(_target, error, conf))
				fill_error_errno(conf, error);
		}
	}
	else {
		// 	=====> Location has a return
		std::map<std::string, std::string>::iterator it = conf.location[location].find("return");
		if (it != conf.location[location].end())
			redirection(it->second, error, location, conf); // A FAIRE : GERER CAS D ERREUR >> TESTER PARSING CONF RETURN
		// 	=====> Request is a directory (end with a "/")		
		else if (is_dir(_target))
			uri_directory(conf, location, error);
		//  =====> Request isn't a directory
		else if (conf.location[location].find("cgi_extension") != conf.location[location].end())
		{
			std::cout << "ENTER IN CGI" << std::endl;
			handle_cgi(conf, location);
		}
		else
			if (!open_targetfile(_target, error, conf))
				fill_error_errno(conf, error);
	}
	// handle_cookies();
	setStatus(RD_TO_SEND);
}

bool	Request::open_targetfile(std::string & target, ErrorPages & error, t_conf &conf) {
	
	std::ifstream     file;
	
	file.open(target.data()); // OK NO LEAK + MEMMORY ET ERROR SET SELON ERRNO
	if (file.is_open()) {
		response.setBody(file);
		response.setStatus("200", " OK");
		std::string	type = extract_extension(target);
		if (type.empty())
			type = "octet-stream";
		if (!response.setContent_type(type)) {
			response.reinitBody();
			error.fill_error(response, "406", conf);
		}
		return (true);
	}
	return (false);
}

bool	Request::is_dir(std::string const &path) {
	struct stat buf;
	if (stat(path.data(), &buf) == -1) // NO LEAK MEMMORY + FD ===> erreur not found renvoyee
		return (false);
	if (S_ISDIR(buf.st_mode))
		return (true);
	return (false);
}

/* Le champs index a la  priorite sur autoindex si les deux sont presents */
void	Request::uri_directory(t_conf &conf, ErrorPages &error) {
	
	for (std::vector<std::string>::iterator it = conf.files_vect.begin(); it != conf.files_vect.end(); it++) {
		std::string tmp = _target + *it;
		if (open_targetfile(tmp, error, conf)) // =====> open le 1er index valide
			return ;
	}
	if (conf.autoindex == "on") // =====> Autoindex on
		return (build_index(conf, error), (void)0);
	error.fill_error(response, "404", conf);
}

/* Le champs index a la  priorite sur autoindex si les deux sont presents */
void	Request::uri_directory(t_conf &conf, std::string &location, ErrorPages &error) {
	
	std::map<std::string, std::string>::iterator it = conf.location[location].find("index");
	if (it != conf.location[location].end()) { // =====> Index in location present
		std::vector<std::string> index;
		strtovect(it->second, index, " ");
		for (std::vector<std::string>::iterator jt = index.begin(); jt != index.end(); jt++) {
			std::string tmp = _target + *jt;
			if (open_targetfile(tmp, error, conf)) // =====> open le 1er index valide
				return ;
		}
	}
	it = conf.location[location].find("autoindex"); // =====> Index in location absent or index page doesn't exist
	if (it == conf.location[location].end() || it->second != "on") // =====> Autoindex off or absent
		return (error.fill_error(response, "404", conf), (void)0);
	else // =====> Autoindex on
		build_index(conf, error);
}

/*  Quand Autoindex est on : Cree la page html de l'index */
void	Request::build_index(t_conf &conf, ErrorPages &error) {
	
	DIR *tmp = opendir(_target.data()); // OK NO LEAKS MEMMORY + FD ====> ERROR SET SELON ERRNO
	dirent *directory;
	
	if (!tmp)
		return (fill_error_errno(conf, error), (void) 0);
	response.setBody("<!DOCTYPE html>\n<html>\n<head>\n<title>Index</title>\n</head>\n<body>\n<h3>Index</h3>\n");
	while (1) {
		directory = readdir(tmp);
		if (!directory) // OK NO LEAKS MEMMORY + FD
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

int	extract_name(std::vector<std::string> & array, std::string & name) // ATTENTION APPARTIENT PAS A REQUEST.HPP
{
	std::string	extract;
	
	for (std::vector<std::string>::iterator it = array.begin(); it != array.end(); it++)
	{
		if ((*it).find("Content-Disposition: form-data") != (*it).npos && (*it).find("filename=") != (*it).npos)
		{
			extract = *it;
			break ;
		}
	}
	extract.erase(0, extract.rfind('=') + 1);
	if (extract == "")
		return (-1);
	name = extract.substr(1, extract.rfind('"') - 1);
	return (0);
}

void	recursive_name(std::string & filename, std::string name, int count)  // ATTENTION APPARTIENT PAS A REQUEST.HPP
{
	if (access(name.c_str(), F_OK) == -1)
		filename = name;
	else
	{
		char c = ++count + '0';
		std::string ext = "()";
		if (count < 2)
			recursive_name(filename, name.insert(name.rfind('.'), ext.insert(1, 1, c)), count);
		else
		{
			name[name.rfind('.') - 2] = c;
			recursive_name(filename, name, count);
		}
	}
}

int	Request::set_filename(std::vector<std::string> & array, std::string & filename)
{
	std::string	name;
	int			count = 0;
	if (extract_name(array, name) != -1)
	{
		recursive_name(filename, name, count);
	}
	else
		return (-1);
	return (0);
}

void	Request::remove_boundaries(std::deque<unsigned char> & copy)
{
	std::vector<std::string> vec;
	std::string str = "";
	int			count = 0;
	std::deque<unsigned char>::iterator start = body_deque.begin();
	std::deque<unsigned char>::iterator end;
	for (std::deque<unsigned char>::iterator it = body_deque.begin(); it != body_deque.end(); it++)
	{
		str += *it;
		if (*it == '\n')
		{
			end = it;
			vec.push_back(str);
			if (str == "--" + boundary + "\r\n" || str == "--" + boundary + "--" + "\r\n")
				start = it + 1;
			else if (count > 0 && (vec[count - 1] == "--" + boundary + "\r\n" || vec[count - 1] == "--" + boundary + "--" + "\r\n"))
				start = it + 1;
			else if (count > 1 && (vec[count - 2] == "--" + boundary + "\r\n" || vec[count - 2] == "--" + boundary + "--" + "\r\n"))
				start = it + 1;
			else if (count > 2 && (vec[count - 3] == "--" + boundary + "\r\n" || vec[count - 3] == "--" + boundary + "--" + "\r\n"))
				start = it + 1;
			else
			{
				for (std::deque<unsigned char>::iterator its = start; its != end; its++)
					copy.push_back(*its);
				start = it;
			}
			str = "";
			count++;
		}
	}
}

void	Request::build_file(std::vector<std::string> & array)
{
	std::string	filename;
	std::string end = "--" + boundary + "--\n";
	std::deque<unsigned char> copy;
	if (set_filename(array, filename) != -1)
	{
		std::ofstream file(filename.c_str(), std::ofstream::out);
		remove_boundaries(copy);
		for (std::deque<unsigned char>::iterator it = copy.begin(); it != copy.end(); it++)
			file << (*it);
	}
}

void	build_array(std::vector<std::string> & array, std::string & str_body)
{
	while (1)
	{
		std::string	sub = str_body.substr(0, str_body.find('\n') + 1);
		array.push_back(sub);
		str_body.erase(0, str_body.find('\n') + 1);
		if (str_body.find('\n') == str_body.npos)
		{
			array.push_back(str_body);
			break ;
		}
	}
}

void	Request::extract_body_upload(std::vector<std::string> & array)
{
	std::string	str_body = "";
	std::deque<unsigned char> copy = body_deque;

	for (std::deque<unsigned char>::iterator it = copy.begin(); it != copy.end(); it++)
		str_body += (*it);	
	build_array(array, str_body);
}

void	Request::check_request(/* int socket_fd,  */t_conf &conf, ErrorPages &error) {

	// modifier nom si deja present
	// ne pas changer contenu du ficher
	std::vector<std::string>	array;

	if (content_type == "multipart/form-data")
	{
		// std::cout << "Request::check_request 0\n";
		extract_body_upload(array);
		// std::cout << "Request::check_request 1\n";
		if (array[0] != "--" + boundary + "\r\n" || array[array.size() - 2] != "--" + boundary + "--\r\n")
			fill_significant_error("400", error, conf);
		// std::cout << "Request::check_request 2\n";
	}	
	if (miss_length && !body.empty() && transfer_encoding != "chunked")
		fill_significant_error("411", error, conf);
	if (content_length < 0 || (static_cast<size_t>(content_length) != body.length() && transfer_encoding != "chunked" && content_type != "multipart/form-data"))
		fill_significant_error("400", error, conf);
	if (content_type == "multipart/form-data") {
		if (static_cast<size_t>(content_length) != body_deque.size())
			fill_significant_error("400", error, conf);
		if (method != "POST")
			fill_significant_error("403", error, conf);
		build_file(array); // A DEPLACER
	}
	if (version != "HTTP/1.1")
		fill_significant_error("405", error, conf);
	if (content_length > conf.limit_body_size)
		fill_significant_error("413", error, conf);
	if (media.size() > 0 && !media_request_allowed())
		fill_significant_error("415", error, conf);
	// std::cout << "Request::check_request 3\n";
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
}

void	Request::cgi_parse_uri() {
    
    size_t found = uri.find('?');

	if (found != std::string::npos) {
		_target = uri.substr(0, found);
		if (found < uri.length() - 1)
			_query_string = uri.substr(found + 1);
	}
	else if (method == "POST") {
		_target = uri;
		_query_string = body;
	}
	else
		_target = uri;
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
}

void	Request::setIp_socket(in_addr_t s_addr) {

	socket_s_addr = s_addr;
	recover_ip_socket();
}


/* Parse first line, host and port */
void	Request::parse_first_line(/* in_addr_t s_addr, ErrorPages &error */) {
	
	std::string title = "\e[34m";
	std::string reset = "\e[0m";
	method = extract_line(save_buffer, ' ');
	uri = extract_line(save_buffer, ' ');
	version = extract_line(save_buffer, '\r');
	host = extract_elem("Host:", "\r", save_buffer, "");
	parse_host();
	std::cout << title << "*********** REQUEST **********" << std::endl;
	std::cout << title << method << " " << uri << " " << version << std::endl;
	std::cout << "******************************" << reset << std::endl;
	if (method.empty() || version.empty() || uri.empty() || host.empty() || port.empty())
		throw (RequestException("400"));
}

bool		Request::body_present() {
	
	if (save_buffer.find("\r\n\r\n") != std::string::npos)
		return (true);
	return (false);
}

// TESTER AVEC MAX INT + 1 OU MIN INT -1
int	Request::ft_shextodec(std::string & s) {
	
	std::istringstream iss(s);
	int n;

	iss >> std::hex >> n;
	if (!iss.eof())
		return (-1);
	return (n);
}

int Request::extract_chunked_body(std::string &s) {
	
	std::string tmp;
	std::string new_body;
	int length = -1;
	
	if (s.empty()) {
			return (-2);
	}
	for (int i = 0; i < 2; i++) {
		tmp = getline(s, "\r\n");
		if (tmp.empty()) {
			if (i == 0) {
				if (s.empty())
					return (-2);
				throw (RequestException("400"));
			}
			else if (length == 0) 
				return (0);
			throw (RequestException("400"));
		}
		if (i == 0) {
			length = ft_shextodec(tmp);
			if (length == -1)
				throw (RequestException("400"));
		}
		else {
			if (tmp.length() != static_cast<size_t>(length) || s != "\r\n")
				throw (RequestException("400"));
			body += tmp;
			return (length);
		}
	}
	return (-2);
}

void	Request::parse_chunk_body() {
	
	std::string tmp = extract_body(save_buffer);
	int chunk = extract_chunked_body(tmp);
	if (chunk == 0)
		status = RD_TO_RESPOND;
	else
		status = READING;
}

void	Request::parse_upload_body(std::string &body) {
	
	for (std::string::iterator it = body.begin(); it != body.end(); it++) {
		body_deque.push_back(*it);
	}
	if (body_deque.size() == static_cast<size_t>(content_length))
		status = RD_TO_RESPOND;
	else
		status = READING;
}

void	Request::parse_body() {
	
	body = extract_body(save_buffer);
	if (body.empty()) {
		body = "";
	}
}

void	Request::handle_multi_length() {
	
	std::string tmp;
	int n;
	
	while (1) {
		tmp = extract_elem("Content-Length:", "\r", save_buffer, "");
		if (tmp.empty())
			return ;
		try {
			n = ft_stoi(tmp); 
			if (n != content_length) {
				content_length = -1;
				return ;
			}
		}
		catch (std::exception & e) {
			std::string err = e.what();
			if (err == "exit")
				throw ;
			else {
				content_length = -1;
				return ;
			}
		}	
	}
}

void	Request::extract_boundary() {
		content_type = extract_elem("Content-Type:", "\r", save_buffer, "");
		try {
			boundary = content_type.substr(content_type.find("; boundary=") + 11);
			content_type.erase(content_type.find("; boundary="));
		}
		catch(std::exception & e) {
			std::string err = e.what();
			if (err == "exit")
				throw ;
		}
}

void	Request::parse_headers() {
	
	std::string tmp;
	
	agent = extract_elem("User-Agent:", "\r", save_buffer, "");
	tmp = extract_elem("Accept:", "\r", save_buffer, "");
	if (!tmp.empty())
		parse_media(tmp);
	connection = extract_elem("Connection:", "\r\n", save_buffer, "keep-alive");
	tmp = extract_elem("Content-Length:", "\r", save_buffer, "");
	if (tmp.empty()) {
		miss_length = true;
		content_length = 0;
	}
	else {
		miss_length = false;
		try { content_length = ft_stoi(tmp); }
		catch (std::exception & e) {
			std::string err = e.what();
			if (err == "exit")
				throw;
			content_length = -1;
			}
	}
	handle_multi_length();
	if (content_type.empty()) {
		extract_boundary();
		if (content_type == "multipart/form-data" && boundary.empty())
			throw (RequestException("400"));
	}
	transfer_encoding = extract_elem("Transfer-Encoding:", "\r", save_buffer, "");
}

void	Request::parse_request() {
	
	if (save_buffer.find("\r\n\r\n\r\n") != std::string::npos) {
		extract_boundary();
		if (content_type != "multipart/form-data" || content_type.find(";") != std::string::npos || boundary.empty())
			throw (RequestException("400"));
	}
	parse_first_line();
	parse_headers();
	if (transfer_encoding == "chunked")
		parse_chunk_body();
	if (content_type == "multipart/form-data") {
		std::string tmp = extract_body(save_buffer);
		parse_upload_body(tmp);
	}
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
	sep_body = buff.find("\r\n\r\n");
	begin = buff.find(elem);
	if (begin == -1 || (sep_body > -1 && begin > sep_body))
		return (nofound);
	end = buff.find(delim, begin);
	std::string tmp (buff.substr(begin, end - begin));
	buff.erase(begin, end - begin);
	if (buff.compare(begin, 4, "\r\n\r\n") != 0 || buff.compare(begin, 6, "\r\n\r\n\r\n") == 0)
		buff.erase(begin, 2);
	return (extract_header(tmp));
}

std::string Request::getline(std::string &src, std::string const & delim) const {
	
	std::string tmp;

	int found = src.find(delim);
	if (found == -1)
		return (src);
	tmp = src.substr(0, found);
	src.erase(0, found + delim.length());
	return (tmp);
}

std::string Request::extract_body(std::string & buff) {
	
	int begin = buff.find("\r\n\r\n");
	if (begin == -1 || static_cast<size_t>(begin + 3) >= buff.length())
		return ("");
	std::string tmp (buff.substr(begin + 4, buff.length() - begin - 4));
	if (transfer_encoding != "chunked" && content_type != "multipart/form-data") {
		int found;
		while (1) {
			found = tmp.find("\r\n");
			if (found == -1)
				break;
			tmp.erase(found, 2);
			}
	}
	return (tmp);
}

std::string Request::extract_extension(std::string const & s) {
	
	int found = s.find_last_of('.');
	if (found == -1 || found == static_cast<int>(s.length() - 1))
		return ("");
	return (s.substr(found + 1));
}

/* ************************************************************************* */
/* ********************************* ERROR ********************************* */
/* ************************************************************************* */	

void	Request::fill_error_errno(t_conf &conf, ErrorPages &error) {
	if (errno == EACCES)
		error.fill_error(response, "403", conf);
	else
		error.fill_error(response, "404", conf);
}

void	Request::fill_error(std::string const &code, ErrorPages &error) {
	error.fill_error(response, code);
	setStatus(RD_TO_SEND);
}

void	Request::fill_significant_error(std::string const &code, ErrorPages &error) {
	error.fill_significant_error(response, code);
	setStatus(ERROR);
}

void	Request::fill_error(std::string const &code, ErrorPages &error, t_conf &conf) {
	error.fill_error(response, code, conf);
	setStatus(RD_TO_SEND);
}

void	Request::fill_significant_error(std::string const &code, ErrorPages &error, t_conf &conf) {
	error.fill_significant_error(response, code, conf);
	setStatus(ERROR);
	throw (ServerException(""));
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

/* ************************************************************************* */	
/* ******************************** A EFFACER ****************************** */
/* ************************************************************************* */	

void	Request::print_response() {
	// response.print();
}
