/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/29 09:18:45 by garance           #+#    #+#             */
/*   Updated: 2024/09/10 11:35:07 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"
// #include <sys/stat.h>
/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Request::Request() {}

Request::Request(char const *buffer, int read, int socket, Server *src_server, ErrorPages *src_error, Media *src_auth_media/* , int id */) {

	time_t	now;
	
	time(&now);
	_t_creation =  localtime(&now);
	_socket_fd = socket;
	_server = src_server;
	_error = src_error;
	_auth_media = src_auth_media;
	_response.setAuthmedia(src_auth_media);
	if (buffer)
		_save_buffer.append(buffer, read);
	_i_conf = -1;
	_body_len = 0;
	_status = NEW;
}

Request::Request(const Request & orig) : _socket_fd(orig._socket_fd), _status(orig._status), _save_buffer(orig._save_buffer) {
	*this = orig;
}

Request::~Request() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */
/* ************************** Assignment Operator  ************************* */

Request &Request::operator=(Request const & rhs) {
	_socket_fd = rhs._socket_fd;
	_socket_s_addr = rhs._socket_s_addr;
	_socket_ip = rhs._socket_ip;
	_status = rhs._status;
	_save_buffer = rhs._save_buffer;
	_method = rhs._method;
	_uri = rhs._uri;
	_version = rhs._version;
	_query_string = rhs._query_string;
	_target = rhs._target;
	_host = rhs._host;
	_port = rhs._port;
	_agent = rhs._agent;
	_media = rhs._media;
	_connection = rhs._connection;
	_content_type = rhs._content_type;
	_transfer_encoding = rhs._transfer_encoding;
	_content_length = rhs._content_length;
	_miss_length = rhs._miss_length;
	_body = rhs._body;
	_response = rhs._response;
	_server = rhs._server;
	_error = rhs._error;
	_auth_media = rhs._auth_media;
	_i_conf = rhs._i_conf;
	_t_creation = rhs._t_creation;
	_boundary = rhs._boundary;
	_body_deque = rhs._body_deque;
	_body_len = rhs._body_len;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

int 		Request::getIconf() const {
	return (_i_conf);
}

int Request::getStatus() const {
	return (_status);
}

tm 		*Request::getT_creation() const {
	return (_t_creation);
}

void 		Request::setT_creation(time_t &now) {
	_t_creation = localtime(&now);
}

int Request::getSocket_fd() const {
	return (_socket_fd);
}

in_addr_t Request::getSocket_s_addr() const {
	return (_socket_s_addr);
}

std::string Request::getSocket_ip() const {
	return (_socket_ip);
}

std::string Request::getHost() const {
	return (_host);
}

std::string Request::getPort() const {
	return (_port);
}

std::string Request::getTransfer_encoding() const {
	return (_transfer_encoding);
}

std::string Request::getContentType() const {
	return (_content_type);
}

std::string Request::getConnection() const {
	return (_connection);
}

void	Request::setStatus(int status) {
	this->_status = status;
}

void	Request::addSave_buffer(char const *buffer, int end) {
	
	_save_buffer.append(buffer, end);
}

/* ************************************************************************* */
/* ****************************** Exceptions ******************************* */
/* ************************************************************************* */

Request::RequestException::RequestException(char const* str) : s(str) {}

const char* Request::RequestException::what() const throw() {		
	
	return (s);
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */

void	Request::send_response(int const &socket_fd) {
	
	_response.setContent_length();
	std::string response_content = _response.build_response(_body_len);
	int fd = send(socket_fd, response_content.c_str(), response_content.size(), MSG_NOSIGNAL);
	if (fd <= 0) {
		_status = CLOSE;
		if (fd < -1)
			throw(ServerException("Fail to write"));
	}
}

//  parse request from client and send back response 
void  Request::handle_request(t_conf &conf, ErrorPages &error)
{
	check_request(conf, error);
	int i = check_exist_method();
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
			delete_action(_target.data(), _socket_fd, conf,  error);
			_response.setStatus("204", error);
			setStatus(RD_TO_SEND);
			return ;
		}
		default : {
			std::string index = find_location(conf, error);
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
std::string Request::look_for_location(t_conf & conf) {
	
	std::string s = "=" + _uri;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	return (look_if_location(_uri, conf)); //on recupere l index de la location :)
}

/*
Cherche si une location = correspondante a la uri existe, sinon appelle look_if_location
*/
std::string Request::look_for_location(std::string &url, t_conf & conf) {
	
	std::string s = "=" + url;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	return (look_if_location(url, conf)); //on recupere l index de la location :)
}

/* Supprime la location et la remplace par la root dans uri */
void	Request::add_path(t_conf & conf, std::string index) {
	
	std::string root;
	
	if (conf.location[index].find("root") == conf.location[index].end())
		root = conf.root_dir;
	else
		root  = conf.location[index]["root"];
	if(strback(index) == '/' && strback(root) != '/')
		root += "/";
	if (index[0] == '=')
		index.erase(0, 1);
	_uri.replace(_uri.find(index), index.length(), root);
}

std::string	Request::find_location(t_conf &conf, ErrorPages &error) {
	std::string index = look_for_location(_uri, conf); // si pas trouve index = ""
	if (index.empty())
	{
		if (conf.root_dir.empty()) {
			fill_error("404", error, conf);
			setStatus(RD_TO_SEND);
			throw (RequestException(""));
		}
		else {
			if (strback(conf.root_dir) == '/')
				_uri.replace(0, 1, conf.root_dir);
			else	
				_uri.insert(0, conf.root_dir);
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
		if (this->_method == method[i])
			break;
	}
	return (i);
}

bool	Request::check_allow_method(t_conf &conf, std::string &index) {
	
	if (conf.location[index].find("allow_methods") != conf.location[index].end()) {
		if (conf.location[index]["allow_methods"].find(this->_method) == std::string::npos)
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
	std::string copy = _uri;

	copy.erase(0, copy.find('/') + 1);
	copy.erase(0, copy.find('/') + 1);
	env = new char* [8];
	for (int i = 0; i < 7; i++)
		env[i] = new char [1024];
	strcpy(env[0], ("REQUEST_METHOD=" + _method).c_str());
	strcpy(env[1], ("QUERY_STRING=" + _query_string).c_str());
	strcpy(env[2], ("SCRIPT_NAME=" + _target).c_str());
	strcpy(env[3], ("SERVER_NAME=" + conf.server_name).c_str());
	strcpy(env[4], ("SERVER_PORT=" + conf.ipv4_port[0]).c_str());
	strcpy(env[5], ("REMOTE_ADDR=" + conf.host).c_str());
	strcpy(env[6], ("HTTP_USER_AGENT=" + _agent).c_str());
	env[7] = NULL;
	return (env);
}

void	Request::get_output(const char *buff, t_conf &conf, char *const argv[], char *const envp[])
{
	int	flag = 0;
	std::string str = buff;
	if (str == "1" || str.empty())
	{
		deleteArgs(argv, envp);
		fill_significant_error("400", *_error, conf);
	}

	while (1)
	{
		size_t pos_cookie = str.find(("Set-Cookie:"));
		size_t pos_loc = str.find(("Location: http"));
		if (pos_cookie != str.npos)
		{
			std::string extract = str.substr(pos_cookie, str.find('\n') - pos_cookie);
			_response.setCookie(extract);
			str.erase(pos_cookie, str.find('\n') - pos_cookie + 1);
			continue ;
		}
		else if (pos_loc != str.npos)
		{
			size_t pos_http = str.find(("http"));
			str.erase(pos_loc, pos_http - pos_loc);
			pos_http = str.find(("http"));
			std::string extract = str.substr(pos_http, str.find('\n') - pos_http);
			_response.setLocation(extract);
			str.erase(pos_http, str.find('\n') - pos_http + 1);
			_response.setStatus("307", " Temporary Redirect");
			flag = 1;
		}
		else
		{
			_response.setBody(str);
			break ;
		}
	}
	if (flag == 0)
		_response.setStatus("200", " OK");
	_response.setContent_type("html");
}

void	Request::setTimerProcess(char *const argv[], char *const envp[], int & timer_pid, t_conf &conf, int & fd, std::ofstream & cgi)
{
	
	timer_pid = fork();
	if (timer_pid == -1)
		fill_significant_error("500", *_error, conf);
	if (timer_pid == 0)
	{
		signal(SIGINT, sighandlerbis);
		close(fd);
		cgi.close();
		_server->close_child_sockets();
		for (int time = 5; time > 0 && _server->getTimeSigint() == false; time--)
			sleep(1);
		if (_server->getTimeSigint() == false)
		{
			deleteArgs(argv, envp);
			delete [] _server->getFds();
			_server->del_all();
			exit(0);
		}
		else
		{
			deleteArgs(argv, envp);
			delete [] _server->getFds();
			_server->del_all();
			exit(255);
		}
	}
	
}

void	Request::setScriptProcess(char const *pathname, char *const argv[], char *const envp[], int & script_pid, t_conf &conf, int & fd, std::ofstream & cgi)
{
	
	script_pid = fork();
	if (script_pid == -1)
		fill_significant_error("500", *_error, conf);
	if (script_pid == 0)
	{
		dup2(fd, 1);
		int	copy_out = dup(STDOUT_FILENO);
		cgi.close();
		close(fd);
		close(copy_out);
		_server->close_child_sockets();
		if (execve(pathname, argv, envp) != 0)
		{
			deleteArgs(argv, envp);
			delete [] _server->getFds();
			_server->del_all();
			exit (1);
		}
	}
}
void		Request::waitPidOutput(int & pid, int & status, int & script_pid, int & timer_pid)
{
	while (1)
	{
		pid = waitpid(WAIT_ANY, &status, 0);
		if (pid == script_pid || pid == timer_pid)
			break ;
	}
}

int	Request::exec_script(char const *pathname, char *const argv[], char *const envp[], t_conf &conf)
{
	int			script_pid;
	int			timer_pid;
	int			pid = 0;
	int			status;
	int			exit_status = 0;
	bool		script_got_killed = false;

	std::ofstream cgi("tmp");
	int fd_cgi = open("tmp", O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fd_cgi == -1)
		fill_significant_error("500", *_error, conf);
	signal(SIGINT, SIG_IGN);
	setScriptProcess(pathname, argv, envp, script_pid, conf, fd_cgi, cgi);
	setTimerProcess(argv, envp, timer_pid, conf, fd_cgi, cgi);
	waitPidOutput(pid, status, script_pid, timer_pid);
	close(fd_cgi);
	cgi.close();
	signal(SIGINT, sighandler);
	if (WIFEXITED(status))
		exit_status = WEXITSTATUS(status);
	if (pid == timer_pid)
	{
		kill(script_pid, SIGKILL);
		script_got_killed = true;
		if (exit_status == 255)
		{
			close(fd_cgi);
			deleteArgs(argv, envp);
			std::remove("tmp");
			throw ServerException("exit");
		}
	}
	else
		kill(timer_pid, SIGKILL);
	if (exit_status == 0)
	{
		std::ifstream 	fd_cgi2("tmp");
		std::string 	buff;
		std::getline(fd_cgi2, buff, '\0');
		std::remove("tmp");
		fd_cgi2.close();
		get_output(buff.c_str(), conf, argv, envp);
		deleteArgs(argv, envp);
	}
	else
	{
		std::remove("tmp");
		deleteArgs(argv, envp);
		fill_significant_error("400", *_error, conf);
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
		fill_significant_error("501", *_error, conf);
}

bool	Request::is_accessible(char const *pathname)
{
	struct stat path_stat;
	stat(pathname, &path_stat);
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

	_str = ("./" + _target);
	char const *pathname = _str.c_str();
	if (_target.empty())
		fill_significant_error("400", *_error, conf);
	struct stat path_stat;
	if (stat(pathname, &path_stat) == -1)
		fill_significant_error("400", *_error, conf);
	std::ifstream	file(pathname, std::ifstream::in);
	if (!is_accessible(pathname) || is_empty(file))
		fill_significant_error("400", *_error, conf);
	file.close();
	check_extension(conf, pathname, index_loc);
	char const * interpreter = define_ext(pathname);
	env = set_env(conf);
    argv = new char * [3];
	argv[0] = (char *) interpreter;
    argv[1] = (char *) pathname;
    argv[2] = NULL;
	exec_script(argv[0], argv, env, conf);
}

bool Request::is_empty(std::ifstream& pFile)
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
			if (!directory) {
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

void	Request::build_response(t_conf &conf, std::string &location, ErrorPages &error) {
	
	if (location.empty()) {
		// 	=====> Server has a return
		if (!conf.ret.empty())
			redirection(conf.ret, error, location, conf);
		// 	=====> upload
		else if (_content_type == "multipart/form-data")
			build_file();
		// 	=====> Request is a directory (end with a "/")
		else if (is_dir(_target))
			uri_directory(conf, error);
		//  =====> Request isn't a directory
		else {
			if (!open_targetfile(_target))
				fill_error_errno(conf, error);
		}
	}
	else {
		// 	=====> Location has a return
		std::map<std::string, std::string>::iterator it = conf.location[location].find("return");
		if (it != conf.location[location].end())
			redirection(it->second, error, location, conf);
		else if (_content_type == "multipart/form-data")
			build_file();
		// 	=====> Request is a directory (end with a "/")		
		else if (is_dir(_target))
			uri_directory(conf, location, error);
		//  =====> Request isn't a directory
		else if (conf.location[location].find("cgi_extension") != conf.location[location].end())
			handle_cgi(conf, location);
		else
			if (!open_targetfile(_target))
				fill_error_errno(conf, error);
	}
	// handle_cookies();
	setStatus(RD_TO_SEND);
}

bool	Request::open_targetfile(std::string &target) {
	
	_file.open(target.data());
	if (_file.is_open()) {
		_body_len = _response.setBody(_file);
		_file.close();
		_response.setStatus("200", " OK");
		std::string	type = extract_extension(target);
		_response.setContent_type(type);
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
		if (open_targetfile(tmp)) // =====> open le 1er index valide
			return ;
	}
	if (conf.autoindex == "on") // =====> Autoindex on
		return (build_index(conf, "", error), (void)0);
	error.fill_error(_response, "404", conf);
}

/* Le champs index a la  priorite sur autoindex si les deux sont presents */
void	Request::uri_directory(t_conf &conf, std::string &location, ErrorPages &error) {
	
	std::map<std::string, std::string>::iterator it = conf.location[location].find("index");
	if (it != conf.location[location].end()) { // =====> Index in location present
		std::vector<std::string> index;
		strtovect(it->second, index, " ");
		for (std::vector<std::string>::iterator jt = index.begin(); jt != index.end(); jt++) {
			std::string tmp = _target + *jt;
			if (open_targetfile(tmp)) // =====> open le 1er index valide
				return ;
		}
	}
	it = conf.location[location].find("autoindex"); // =====> Index in location absent or index page doesn't exist
	if (it == conf.location[location].end() || it->second != "on") // =====> Autoindex off or absent
		return (error.fill_error(_response, "404", conf), (void)0);
	else // =====> Autoindex on
		build_index(conf, location, error);
}

/*  Quand Autoindex est on : Cree la page html de l'index */
void	Request::build_index(t_conf &conf, std::string location, ErrorPages &error) {
	
	DIR *tmp = opendir(_target.data());
	dirent *directory;
	
	if (!tmp)
		return (fill_error_errno(conf, error), (void) 0);
	_body_len += _response.setBody("<!DOCTYPE html>\n<html>\n<head>\n<title>Index</title>\n</head>\n<body>\n<h3>Index</h3>\n");
	while (1) {
		directory = readdir(tmp);
		if (!directory)
			break; // =====> readdir a fini de lister
		std::string link = directory->d_name;
		if (link == "." || link == "..")
			continue;
		if (!location.empty() && location != "/") {
			if (location[0] == '=')
				location.erase(0, 1);
			link = location + "/" + link;
		}
		if (static_cast<int>(directory->d_type) == 4) // =====> Si le lien est un dir = 4 si html =8
			link += "/";
		_body_len += _response.setBody("<p><a href=\"");
		_body_len += _response.setBody(link);
		_body_len += _response.setBody("\">");
		_body_len += _response.setBody(directory->d_name);
		_body_len += _response.setBody("</a></p>\n");
	}
	closedir(tmp);
	_body_len += _response.setBody("</body>");
	_body_len += _response.setBody("</html>");
	_response.setStatus("200", " OK");
	_response.setContent_type("html"); // Type ok : l'index auto genere est html
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
		return (error.fill_error(_response, "508", conf), (void)0);
	redir = "http://" + _host + ":" + _port + redir;
	error.fill_redir(_response, code, redir);
}

/* ************************************************************************* */
/* ******************************** Parsing ******************************** */
/* ************************************************************************* */


bool	Request::media_request_allowed() {
	
	for (std::map<std::string, std::vector<std::string> >::iterator it=_media.begin() ; it != _media.end(); it++) {
		for (std::vector<std::string>::iterator jt = it->second.begin(); jt != it->second.end(); jt++)
			if (_auth_media->is_allow(it->first, *jt))
				return (true);
	}
	return (false);
}

/* ************************************************************************* */
/* ******************************** Upload ********************************* */
/* ************************************************************************* */

int	Request::extract_name(std::string & name)
{
	std::string	extract;
	
	for (std::vector<std::string>::iterator it = _array_upload.begin(); it != _array_upload.end(); it++)
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

void	Request::recursive_name(std::string & file_path, std::string name, int count) 
{
	std::string	path = "./pages/upload_dir/" + name;
	
	if (access(path.c_str(), F_OK) == -1)
		file_path = path;
	else
	{
		char c = ++count + '0';
		std::string ext = "()";
		if (count < 2)
			recursive_name(file_path, name.insert(name.rfind('.'), ext.insert(1, 1, c)), count);
		else
		{
			name[name.rfind('.') - 2] = c;
			recursive_name(file_path, name, count);
		}
	}
}

int	Request::set_filename(std::string & file_path)
{
	std::string	name;
	int			count = 0;
	
	if (extract_name(name) != -1)
	{
		recursive_name(file_path, name, count);
	}
	else
		return (-1);
	return (0);
}

void	Request::remove_boundaries(std::vector<unsigned char> & copy)
{
	std::vector<std::string> vec;
	std::string str = "";
	int			count = 0;
	std::vector<unsigned char>::iterator start = _body_deque.begin();
	std::vector<unsigned char>::iterator end;
	for (std::vector<unsigned char>::iterator it = _body_deque.begin(); it != _body_deque.end(); it++)
	{
		str += *it;
		if (*it == '\n')
		{
			end = it;
			vec.push_back(str);
			if (str == "--" + _boundary + "\r\n" || str == "--" + _boundary + "--" + "\r\n")
				start = it + 1;
			else if (count > 0 && (vec[count - 1] == "--" + _boundary + "\r\n" || vec[count - 1] == "--" + _boundary + "--" + "\r\n"))
				start = it + 1;
			else if (count > 1 && (vec[count - 2] == "--" + _boundary + "\r\n" || vec[count - 2] == "--" + _boundary + "--" + "\r\n"))
				start = it + 1;
			else if (count > 2 && (vec[count - 3] == "--" + _boundary + "\r\n" || vec[count - 3] == "--" + _boundary + "--" + "\r\n"))
				start = it + 1;
			else
			{
				for (std::vector<unsigned char>::iterator its = start; its != end; its++)
					copy.push_back(*its);
				start = it;
			}
			str = "";
			count++;
		}
	}
}

void	Request::build_file()
{
	std::string	file_path;
	std::string end = "--" + _boundary + "--\n";
	std::vector<unsigned char> copy;
	
	if (set_filename(file_path) != -1)
	{
		std::ofstream file(file_path.c_str(), std::ofstream::out);
		remove_boundaries(copy);
		int i = 0;
		for (std::vector<unsigned char>::iterator it = copy.begin(); it != copy.end(); it++) {
			file << (*it);
			i++;
		}
	}
	_response.setStatus("204", *_error);
}

void	Request::build_array()
{
	while (1)
	{
		_sub = _str_body.substr(0, _str_body.find('\n') + 1);
		_array_upload.push_back(_sub);
		_str_body.erase(0, _str_body.find('\n') + 1);
		if (_str_body.find('\n') == _str_body.npos)
		{
			_array_upload.push_back(_str_body);
			break ;
		}
	}
}

void	Request::extract_body_upload()
{
	_copy_upload = _body_deque;

	for (std::vector<unsigned char>::iterator it = _copy_upload.begin(); it != _copy_upload.end(); it++)
		_str_body += (*it);	
	build_array();
}

void	Request::check_request(t_conf &conf, ErrorPages &error) {

	if (_content_type == "multipart/form-data")
	{
		extract_body_upload(/* array_upload */);
		if (_array_upload[0] != "--" + _boundary + "\r\n" || _array_upload[_array_upload.size() - 2] != "--" + _boundary + "--\r\n")
			fill_significant_error("400", error, conf);
	}	
	if (_miss_length && !_body.empty() && _transfer_encoding != "chunked")
		fill_significant_error("411", error, conf);
	if (_content_length < 0 || (static_cast<size_t>(_content_length) != _body.length() && _transfer_encoding != "chunked" && _content_type != "multipart/form-data"))
		fill_significant_error("400", error, conf);
	if (_content_type == "multipart/form-data") {
		if (static_cast<size_t>(_content_length) != _body_deque.size())
			fill_significant_error("400", error, conf);
		if (_method != "POST")
			fill_significant_error("403", error, conf);
	}
	if (_version != "HTTP/1.1")
		fill_significant_error("405", error, conf);
	if (_content_length > conf.limit_body_size)
		fill_significant_error("413", error, conf);
	if (_media.size() > 0 && !media_request_allowed())
		fill_significant_error("415", error, conf);
}

void	Request::recover_ip_socket() {
	
	u_int32_t n = ntohl(_socket_s_addr);
	std::stringstream iss;
	for (int i = 24; i >= 0; i -= 8) {
		int tmp = (n >> i) & 0xFF;
		iss << tmp;
		if (i > 0)
			iss << '.';
	}
	_socket_ip = iss.str();
}

void	Request::cgi_parse_uri() {
    
    size_t found = _uri.find('?');

	if (found != std::string::npos) {
		_target = _uri.substr(0, found);
		if (found < _uri.length() - 1)
			_query_string = _uri.substr(found + 1);
	}
	else if (_method == "POST") {
		_target = _uri;
		_query_string = _body;
	}
	else
		_target = _uri;
}

void	Request::parse_host() {
	
	std::istringstream	iss(_host);
	
	std::getline(iss, _host, ':');
	str_tolower(_host);
	std::getline(iss, _port);
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
	strtomap(s, _media, ",", "/");
}

void	Request::setIp_socket(in_addr_t s_addr) {

	_socket_s_addr = s_addr;
	recover_ip_socket();
}


/* Parse first line, host and port */
void	Request::parse_first_line(/* in_addr_t s_addr, ErrorPages &error */) {
	
	std::string title = "\e[34m";
	std::string reset = "\e[0m";
	
	_method = extract_line(_save_buffer, ' ');
	_uri = extract_line(_save_buffer, ' ');
	_version = extract_line(_save_buffer, '\r');
	_host = extract_elem("Host:", "\r", _save_buffer, "");
	parse_host();
	std::cout << title << "*********** REQUEST **********" << std::endl;
	std::cout << title << _method << " " << _uri << " " << _version << std::endl;
	std::cout << "******************************" << reset << std::endl;
	if (_method.empty() || _version.empty() || _uri.empty() || _host.empty() || _port.empty())
		throw (RequestException("400"));
}

bool		Request::body_present() {
	
	if (_save_buffer.find("\r\n\r\n") != std::string::npos)
		return (true);
	return (false);
}

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
			_body += tmp;
			return (length);
		}
	}
	return (-2);
}

void	Request::parse_chunk_body() {
	
	std::string tmp = extract_body(_save_buffer);
	int chunk = extract_chunked_body(tmp);
	if (chunk == 0)
		_status = RD_TO_RESPOND;
	else
		_status = READING;
}

void	Request::parse_upload_body(std::string &body) {
	
	for (std::string::iterator it = body.begin(); it != body.end(); it++) {
		_body_deque.push_back(*it);
	}
	if (_body_deque.size() == static_cast<size_t>(_content_length))
		_status = RD_TO_RESPOND;
	else
		_status = READING;
}

void	Request::parse_body() {
	
	_body = extract_body(_save_buffer);
	if (_body.empty()) {
		_body = "";
	}
}

void	Request::handle_multi_length() {
	
	std::string tmp;
	int n;
	
	while (1) {
		tmp = extract_elem("Content-Length:", "\r", _save_buffer, "");
		if (tmp.empty())
			return ;
		try {
			n = ft_stoi(tmp); 
			if (n != _content_length) {
				_content_length = -1;
				return ;
			}
		}
		catch (std::exception & e) {
			std::string err = e.what();
			if (err == "exit")
				throw ;
			else {
				_content_length = -1;
				return ;
			}
		}	
	}
}

void	Request::extract_boundary() {
		_content_type = extract_elem("Content-Type:", "\r", _save_buffer, "");
		try {
			_boundary = _content_type.substr(_content_type.find("; boundary=") + 11);
			_content_type.erase(_content_type.find("; boundary="));
		}
		catch(std::exception & e) {
			std::string err = e.what();
			if (err == "exit")
				throw ;
		}
}

void	Request::parse_headers() {
	
	std::string tmp;
	
	_agent = extract_elem("User-Agent:", "\r", _save_buffer, "");
	tmp = extract_elem("Accept:", "\r", _save_buffer, "");
	if (!tmp.empty())
		parse_media(tmp);
	_connection = extract_elem("Connection:", "\r\n", _save_buffer, "keep-alive");
	tmp = extract_elem("Content-Length:", "\r", _save_buffer, "");
	if (tmp.empty()) {
		_miss_length = true;
		_content_length = 0;
	}
	else {
		_miss_length = false;
		try { _content_length = ft_stoi(tmp); }
		catch (std::exception & e) {
			std::string err = e.what();
			if (err == "exit")
				throw;
			_content_length = -1;
			}
	}
	handle_multi_length();
	if (_content_type.empty()) {
		extract_boundary();
		if (_content_type == "multipart/form-data" && _boundary.empty())
			throw (RequestException("400"));
	}
	_transfer_encoding = extract_elem("Transfer-Encoding:", "\r", _save_buffer, "");
}

void	Request::parse_request() {
	
	if (_save_buffer.find("\r\n\r\n\r\n") != std::string::npos) {
		extract_boundary();
		if (_content_type != "multipart/form-data" || _content_type.find(";") != std::string::npos || _boundary.empty())
			throw (RequestException("400"));
	}
	parse_first_line();
	parse_headers();
	if (_transfer_encoding == "chunked")
		parse_chunk_body();
	if (_content_type == "multipart/form-data") {
		std::string tmp = extract_body(_save_buffer);
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
	if (_transfer_encoding != "chunked" && _content_type != "multipart/form-data") {
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
		error.fill_error(_response, "403", conf);
	else
		error.fill_error(_response, "404", conf);
}

void	Request::fill_error(std::string const &code, ErrorPages &error) {
	error.fill_error(_response, code);
	setStatus(RD_TO_SEND);
}

void	Request::fill_significant_error(std::string const &code, ErrorPages &error) {
	error.fill_significant_error(_response, code);
	setStatus(ERROR);
}

void	Request::fill_error(std::string const &code, ErrorPages &error, t_conf &conf) {
	error.fill_error(_response, code, conf);
	setStatus(RD_TO_SEND);
}

void	Request::fill_significant_error(std::string const &code, ErrorPages &error, t_conf &conf) {
	error.fill_significant_error(_response, code, conf);
	setStatus(ERROR);
	throw (ServerException(""));
}

/* ************************************************************************* */
/* ********************************* CLOSE ********************************* */
/* ************************************************************************* */	

void	Request::del_all() {
	
	// array_upload.clear();
	_file.close();
	std::vector<std::string>().swap(_array_upload);
	std::vector<unsigned char>().swap(_copy_upload);
	_media.clear();
}

void	Request::handle_pending_requests(ErrorPages & error, int const &socket) {
	
	_response.reinitBody();
	if (_i_conf > -1)
		error.fill_error(_response, "503", _server->conf[_i_conf]);
	else
		error.fill_error(_response, "503");
	send_response(socket);
	del_all();
}
