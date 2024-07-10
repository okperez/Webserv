/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/07/10 16:18:08 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Response.hpp"
# include "Server.hpp"

typedef struct s_conf t_conf;
class Server;

class Request
{
	private : 
		int socket_fd;
		in_addr_t	socket_s_addr;
		std::string	socket_ip;
		int status;
		std::string		save_buffer; //Pour sauvegarder le buffer si pas entierement lu
		// std::string		response_content;
		
		 // REQUEST line
		std::string   method;   // HTTP method (ex: GET)
		std::string   target;   // Request target (ex: index.html)
		std::string   version;  // HTTP version (ex: HTTP/1.1)

		// POUR CGI : PARSING TARGET
		std::string   _query_string;
		std::string   _script_name;

		
		// HEADER section
		std::string		host;      		// Header that specifies the server's host and port
		std::string		port;      		// Header that describes the pport used
		std::string   	agent;    		// Header that describes the client's user agent
		// std::string   media;    		// Header that specifies which media types the client can accept
		std::string		connection;    	// Header that specifies if we have to close the connection or keeping it alive
		std::string		content_type;
		int				content_length; // Header that specifies the length of the body
		bool			miss_length;
		std::string		body;

		// Response
		Response	response;
		
		Server		*server;
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
		Request(char const *buffer, int read, int socket, Server * src_server);
		Request(const Request & orig);
		~Request();
		Request &operator=(Request const & rhs);

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		std::string getSave_buffer() const; //A EFFACER
		
		// Response 	&getsetResponse();
		int 		getIconf() const;
		int 		getStatus() const;
		int 		getSocket_fd() const;
		in_addr_t 	getSocket_s_addr() const;
		std::string getSocket_ip() const;
		std::string getHost() const;
		std::string getPort() const;
		std::string getConnection() const;
		void		addSave_buffer(const char *buffer);
		void		setStatus(int status);
		
        /* ***************************************************************** */
        /* **************************** Parsing **************************** */
        /* ***************************************************************** */

		void		parse_host();
		void		cgi_parse_target();
		bool		check_request(int socket_fd, t_conf &conf, ErrorPages &error);
		void		recover_ip_socket();
		void		parse_request(in_addr_t s_addr);
		
        /* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
		
        int  		handle_request(int socket_fd, t_conf &conf, ErrorPages &error);
		void		send_response(int socket_fd);
		
		/* ***************************************************************** */
		/* *************************** Location **************************** */
		/* ***************************************************************** */
		
		std::string look_for_location(t_conf & conf);
		std::string look_for_location(std::string &uri, t_conf & conf);
		std::string look_if_location(std::string &target, t_conf & conf);
		void		add_path(t_conf & conf, std::string &index);
		
		/* ***************************************************************** */
		/* ***************************** Method **************************** */
		/* ***************************************************************** */
		
		int			check_exist_method();
		bool		check_allow_method(t_conf &conf, std::string &index);
		
		/* ***************************************************************** */
		/* ****************************** GET ****************************** */
		/* ***************************************************************** */
		
		void		build_response(int socket_fd, t_conf &conf, std::string &location, ErrorPages &error);
		bool		open_targetfile(std::string & target);
		bool		is_dir(std::string const &path);
		void		target_directory(t_conf &conf, ErrorPages &error);
		void		target_directory(t_conf &conf, std::string &location, ErrorPages &error);
		void		build_index();
		bool		is_valid_code(std::string const code);
		bool		is_loop(std::string &redir, std::string const &location, t_conf & conf);
		void		redirection(std::string const &ret, ErrorPages &error, std::string const &location, t_conf & conf);
		

		/* ***************************************************************** */
		/* ****************************** CGI ****************************** */
		/* ***************************************************************** */
		void	handle_cgi(t_conf & conf);
		char**	set_env(t_conf & conf);
		int		exec_script(char const *pathname, char *const argv[], char *const envp[]);
		bool	is_accessible(char const *target);

		/* ***************************************************************** */
		/* ****************************** POST ***************************** */
		/* ***************************************************************** */
		
		/* ***************************************************************** */
		/* ***************************** Utils ***************************** */
		/* ***************************************************************** */

		std::string extract_line(std::string & buff, char delim) const;
		std::string extract_header(std::string & buff) const;
		std::string extract_elem(std::string const &elem, std::string const &delim, std::string & buff, std::string const & nofound) const;
		std::string extract_body(std::string & buff);

		/* ***************************************************************** */
		/* ***************************** CLOSE ***************************** */
		/* ***************************************************************** */	
		
		void	handle_pending_requests(ErrorPages & error);
} ;

#endif