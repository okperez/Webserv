/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/08/21 12:36:49 by galambey         ###   ########.fr       */
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
		int 			socket_fd;
		in_addr_t		socket_s_addr;
		std::string		socket_ip;
		int 			status;
		std::string		save_buffer; //Pour sauvegarder le buffer si pas entierement lu
		// std::string		response_content;
		
		// REQUEST line
		std::string   	method;   // HTTP method (ex: GET)
		std::string   	uri;   // Request uri (ex: index.html)
		std::string   	version;  // HTTP version (ex: HTTP/1.1)

		// FOR TIME_OUT
		struct tm		*t_creation;

		// POUR CGI : PARSING TARGET
		std::string   	_query_string;
		std::string   	_target;

		
		// HEADER REQUEST section
		std::string											host;      			// Header that specifies the server's host and port
		std::string											port;      			// Header that describes the pport used
		std::string   										agent;    			// Header that describes the client's user agent
		std::map<std::string, std::vector<std::string> >	media;    			// Header that specifies which media types the client can accept
		std::string											connection;    		// Header that specifies if we have to close the connection or keeping it alive
		std::string											content_type;		// UTILISE??
		std::string											transfer_encoding;	// for chunked request
		int													content_length; 	// Header that specifies the length of the body
		bool												miss_length;
		std::string											body;

		// RESPONSE
		Response	response;
		
		Server		*server;
		ErrorPages		*error;
		Media		*auth_media;
		int 		i_conf;
		// size_t        lenght;
		
		Request();
		
	class   RequestException : public std::exception
	{
    public:
    
    char const * s;
    RequestException(char const* str) : s(str) {}
    virtual const char* what() const throw()
    {
        return (s);
    }
	};

	public :

		Request(char const *buffer, /* int read, */ int socket, Server *src_server, ErrorPages *src_error, Media *src_auth_media/* , int id */);
		Request(const Request & orig);
		~Request();
		Request &operator=(Request const & rhs);

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		std::string getSave_buffer() const; //A EFFACER
		std::string getBody() const; //A EFFACER
		
		// Response 	&getsetResponse();
		int 		getIconf() const;
		int 		getStatus() const;
		tm 			*getT_creation() const;
		void 		setT_creation(time_t &now);
		int 		getSocket_fd() const;
		size_t 		getSave_buffer_length() const;
		in_addr_t 	getSocket_s_addr() const;
		std::string getSocket_ip() const;
		std::string getHost() const;
		std::string getPort() const;
		std::string getConnection() const;
		std::string getTransfer_encoding() const;
		void		addSave_buffer(const char *buffer);
		void		setStatus(int status);
		void		setIp_socket(in_addr_t s_addr);		
		
        /* ***************************************************************** */
        /* **************************** Parsing **************************** */
        /* ***************************************************************** */

		void		parse_host();
		void		parse_media(std::string &s);
		void		cgi_parse_uri();
		bool		media_request_allowed();
		bool		check_request(/* int socket_fd,  */t_conf &conf, ErrorPages &error);
		void		recover_ip_socket();
		void		parse_first_line(/* in_addr_t s_addr, ErrorPages &error */);
		bool		body_present();
		int 		extract_chunked_body(std::string &s);
		void		handle_multi_length();
		void		parse_body();
		void		parse_header();
		
        /* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
		
        void  		handle_request(/* int socket_fd,  */t_conf &conf, ErrorPages &error);
		void		send_response(int socket_fd);
		
		/* ***************************************************************** */
		/* *************************** Location **************************** */
		/* ***************************************************************** */
		
		std::string look_for_location(t_conf & conf);
		std::string look_for_location(std::string &url, t_conf & conf);
		std::string look_if_location(std::string &uri, t_conf & conf);
		void		add_path(t_conf & conf, std::string &index);
		std::string	find_location(t_conf &conf, ErrorPages &error);
		
		/* ***************************************************************** */
		/* ***************************** Method **************************** */
		/* ***************************************************************** */
		
		int			check_exist_method();
		bool		check_allow_method(t_conf &conf, std::string &index);
		
		/* ***************************************************************** */
		/* **************************** DELETE ***************************** */
		/* ***************************************************************** */
		
		void	delete_action(int socket_fd, t_conf &conf, ErrorPages &error);	
		// void	delete_action(/*int socket_fd, */t_conf &conf, ErrorPages &error);	
		
		/* ***************************************************************** */
		/* ****************************** GET ****************************** */
		/* ***************************************************************** */
		
		void		build_response(int socket_fd, t_conf &conf, std::string &location, ErrorPages &error);
		bool		open_targetfile(std::string & uri, ErrorPages & error, t_conf &conf);
		bool		is_dir(std::string const &path);
		void		uri_directory(t_conf &conf, ErrorPages &error);
		void		uri_directory(t_conf &conf, std::string &location, ErrorPages &error);
		void		build_index(t_conf &conf, ErrorPages &error);
		bool		is_valid_code(std::string const code);
		bool		is_loop(std::string &redir, std::string const &location, t_conf & conf);
		void		redirection(std::string const &ret, ErrorPages &error, std::string const &location, t_conf & conf);
		

		/* ***************************************************************** */
		/* ****************************** CGI ****************************** */
		/* ***************************************************************** */
		void			handle_cgi(t_conf & conf, std::string & index_loc);
		char**			set_env(t_conf & conf);
		int				exec_script(char const *pathname, char *const argv[], char *const envp[]);
		bool			is_accessible(char const *uri);
		void			check_extension(t_conf & conf, std::string uri, std::string & index_loc);
		void			get_output(char *buff);
		char**			setArg(char const *pathname, char const *interpreter);

		/* ***************************************************************** */
		/* **************************** Cookies **************************** */
		/* ***************************************************************** */
		void		handle_cookies(void);
		void		setSession(void);
		void		create_data_file(void);
		void		setTimestamp(std::ofstream & data);

		/* ***************************************************************** */
		/* ****************************** POST ***************************** */
		/* ***************************************************************** */
		
		/* ***************************************************************** */
		/* ***************************** Utils ***************************** */
		/* ***************************************************************** */

		int					ft_shextodec(std::string & s);
		std::string 		extract_line(std::string &buff, char delim) const;
		std::string 		extract_header(std::string &buff) const;
		std::string 		extract_elem(std::string const &elem, std::string const &delim, std::string &buff, std::string const & nofound) const;
		std::string 		getline(std::string &src, std::string const & delim) const;
		std::string 		extract_body(std::string &buff);
		static std::string 	extract_extension(std::string const & s);

		/* ***************************************************************** */
		/* ***************************** ERROR ***************************** */
		/* ***************************************************************** */	

		void	fill_error_errno(t_conf &conf, ErrorPages &error);

		void	fill_error(std::string const &code, ErrorPages &error);
		void	fill_significant_error(std::string const &code, ErrorPages &error);
		void	fill_error(std::string const &code, ErrorPages &error, t_conf &conf);
		void	fill_significant_error(std::string const &code, ErrorPages &error, t_conf &conf);

		/* ***************************************************************** */
		/* ***************************** CLOSE ***************************** */
		/* ***************************************************************** */	
		
		void	handle_pending_requests(ErrorPages & error, int &socket);

		/* ************************************************************************* */	
		/* ******************************** A EFFACER ****************************** */
		/* ************************************************************************* */	

		void	print_response();

};

void	setExtensions(std::map<std::string, char const *> &extensions);
void	deleteArgs(char *const argv[], char *const envp[]);
void	deleteArr(char *const env[], int index);
bool	is_empty(std::ifstream& pFile);

#endif