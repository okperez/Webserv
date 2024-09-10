/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/09/10 11:19:45 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Response.hpp"
# include "Server.hpp"

typedef struct s_conf t_conf;
class Server;
class Response;

class Request
{
	private : 
		int 						_socket_fd;
		in_addr_t					_socket_s_addr;
		std::string					_socket_ip;
		int 						_status;
		std::string					_save_buffer; //Pour sauvegarder le buffer si pas entierement lu
		
		// REQUEST line
		std::string   				_method;   // HTTP method (ex: GET)
		std::string   				_uri;   // Request uri (ex: index.html)
		std::string   				_version;  // HTTP version (ex: HTTP/1.1)

		// FOR TIME_OUT
		struct tm					*_t_creation;

		// POUR CGI : PARSING TARGET
		std::string   				_query_string;
		std::string   				_target;

		
		// HEADER REQUEST section
		std::string					_host;      			// Header that specifies the server's host and port
		std::string					_port;      			// Header that describes the pport used
		std::string   				_agent;    			// Header that describes the client's user agent
		std::string					_connection;    		// Header that specifies if we have to close the connection or keeping it alive
		std::string					_content_type;		// UTILISE??
		std::string					_transfer_encoding;	// for chunked request
		int							_content_length; 	// Header that specifies the length of the body
		bool						_miss_length;
		std::string					_body;
		std::string					_boundary;
		std::vector<std::string>	_array_upload;
		std::vector<unsigned char> 	_copy_upload;
		std::string 				_str_body;
		std::string					_sub;
		std::map<std::string, std::vector<std::string> >	_media;    			// Header that specifies which media types the client can accept
		
		// RESPONSE
		Response					_response;
		size_t						_body_len;
		std::vector<unsigned char>	_body_deque;
		
		// LINKS
		Server						*_server;
		ErrorPages					*_error;
		Media						*_auth_media;
		int 						_i_conf;
		
		// UTILS
		std::string					_str;
		std::ifstream     			_file;
		
		Request();
		
		/* ***************************************************************** */
		/* **************************** Exception ************************** */
		/* ***************************************************************** */

		class   RequestException : public std::exception
		{
			public:
			
			char const * s;
	
			RequestException(char const* str);
			virtual const char* what() const throw();
		};

		/* ***************************************************************** */
        /* **************************** Parsing **************************** */
        /* ***************************************************************** */

		void		parse_host();
		void		parse_media(std::string &s);
		void		cgi_parse_uri();
		bool		media_request_allowed();
		void		check_request(t_conf &conf, ErrorPages &error);
		void		recover_ip_socket();
		void		parse_first_line();
		void		parse_chunk_body();
		void		handle_multi_length();
		void		extract_boundary();
		void		parse_headers();
		void		extract_body_upload(/* std::vector<std::string> & array */);
		void		build_array(/* std::vector<std::string> & array, std::string & str_body */);
		void		build_file(/* std::vector<std::string> & array */);
		int			set_filename(/* std::vector<std::string> & array,  */std::string & filename);
		void		remove_boundaries(std::vector<unsigned char> & copy);
		int			extract_name(/* std::vector<std::string> & array,  */std::string & name);
		void		recursive_name(std::string & filename, std::string name, int count);
		
		/* ***************************************************************** */
		/* *************************** Location **************************** */
		/* ***************************************************************** */
		
		std::string look_for_location(t_conf & conf);
		std::string look_for_location(std::string &url, t_conf & conf);
		std::string look_if_location(std::string &uri, t_conf & conf);
		void		add_path(t_conf & conf, std::string index);
		std::string	find_location(t_conf &conf, ErrorPages &error);
		
		/* ***************************************************************** */
		/* ***************************** Method **************************** */
		/* ***************************************************************** */
		
		int			check_exist_method();
		bool		check_allow_method(t_conf &conf, std::string &index);
		
		/* ***************************************************************** */
		/* **************************** DELETE ***************************** */
		/* ***************************************************************** */
		
		void		delete_action(const char *target, int socket_fd, t_conf &conf, ErrorPages &error);	

		/* ***************************************************************** */
		/* ****************************** GET ****************************** */
		/* ***************************************************************** */
		
		void		build_response(t_conf &conf, std::string &location, ErrorPages &error);
		bool		open_targetfile(std::string &target);
		bool		is_dir(std::string const &path);
		void		uri_directory(t_conf &conf, ErrorPages &error);
		void		uri_directory(t_conf &conf, std::string &location, ErrorPages &error);
		void		build_index(t_conf &conf, std::string location, ErrorPages &error);
		bool		is_loop(std::string &redir, std::string const &location, t_conf & conf);
		void		redirection(std::string const &ret, ErrorPages &error, std::string const &location, t_conf & conf);
				
		/* ***************************************************************** */
		/* ****************************** CGI ****************************** */
		/* ***************************************************************** */
		void		handle_cgi(t_conf & conf, std::string & index_loc);
		char**		set_env(t_conf & conf);
		int			exec_script(char const *pathname, char *const argv[], char *const envp[], t_conf &conf);
		bool		is_accessible(char const *uri);
		void		check_extension(t_conf & conf, std::string uri, std::string & index_loc);
		void		get_output(const char *buff, t_conf &conf, char *const argv[], char *const envp[]);
		void		setScriptProcess(char const *pathname, char *const argv[], char *const envp[], int & script_pid, t_conf &conf, int & fd, std::ofstream & cgi);
		void		setTimerProcess(char *const argv[], char *const envp[], int & timer_pid, t_conf &conf, int & fd, std::ofstream & cgi);
		void		waitPidOutput(int & pid, int & status, int & script_pid, int & timer_pid);

		/* ***************************************************************** */
		/* **************************** Cookies **************************** */
		/* ***************************************************************** */

		/* ***************************************************************** */
		/* ***************************** Utils ***************************** */
		/* ***************************************************************** */

		int			ft_shextodec(std::string & s);
		std::string extract_line(std::string &buff, char delim) const;
		std::string extract_header(std::string &buff) const;
		std::string extract_elem(std::string const &elem, std::string const &delim, std::string &buff, std::string const & nofound) const;
		std::string getline(std::string &src, std::string const & delim) const;
		std::string extract_body(std::string &buff);
		bool		is_empty(std::ifstream& pFile);

		/* ***************************************************************** */
		/* ***************************** ERROR ***************************** */
		/* ***************************************************************** */	

		void		fill_error_errno(t_conf &conf, ErrorPages &error);
		void		fill_error(std::string const &code, ErrorPages &error, t_conf &conf);
		void		fill_significant_error(std::string const &code, ErrorPages &error, t_conf &conf);
		
		/* ***************************************************************** */
		/* ***************************** CLOSE ***************************** */
		/* ***************************************************************** */	
		
		void		del_all();

	public :

		Request(char const *buffer, int read, int socket, Server *src_server, ErrorPages *src_error, Media *src_auth_media/* , int id */);
		Request(const Request & orig);
		~Request();
		Request &operator=(Request const & rhs);

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		int 		getIconf() const;
		int 		getStatus() const;
		tm 			*getT_creation() const;
		void 		setT_creation(time_t &now);
		int 		getSocket_fd() const;
		in_addr_t 	getSocket_s_addr() const;
		std::string getSocket_ip() const;
		std::string getHost() const;
		std::string getPort() const;
		std::string getConnection() const;
		std::string getTransfer_encoding() const;
		std::string getContentType() const;
		void		addSave_buffer(char const *buffer, int end);
		void		setStatus(int status);
		void		setIp_socket(in_addr_t s_addr);		
		
        /* ***************************************************************** */
        /* **************************** Parsing **************************** */
        /* ***************************************************************** */

		bool		body_present();
		int 		extract_chunked_body(std::string &s);
		void		parse_upload_body(std::string & body);
		void		parse_body();
		void		parse_request();
		
        /* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
		
        void  		handle_request(/* int socket_fd,  */t_conf &conf, ErrorPages &error);
		void		send_response(int const &socket_fd);

		/* ***************************************************************** */
		/* ***************************** Utils ***************************** */
		/* ***************************************************************** */

		static std::string 	extract_extension(std::string const & s);

		/* ***************************************************************** */
		/* ***************************** ERROR ***************************** */
		/* ***************************************************************** */	

		void		fill_error(std::string const &code, ErrorPages &error);
		void		fill_significant_error(std::string const &code, ErrorPages &error);

		/* ***************************************************************** */
		/* ***************************** CLOSE ***************************** */
		/* ***************************************************************** */	
		
		void		handle_pending_requests(ErrorPages & error, int const &socket);
};

#endif