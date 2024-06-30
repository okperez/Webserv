/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/06/30 09:22:02 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "include.hpp"

typedef struct s_conf t_conf;

class Request
{
	private : 
		int socket_fd;
		int status;
		std::string		save_buffer; //Pour sauvegarder le buffer si pas entierement lu
		std::string		response_content;
		
		 // REQUEST line
		std::string   method;   // HTTP method (ex: GET)
		std::string   target;   // Request target (ex: index.html)
		std::string   version;  // HTTP version (ex: HTTP/1.1)


		// HEADER section
		std::string   host;     // Header that specifies the server's host and port
		std::string   port;     // Header that describes the pport used
		std::string   agent;    // Header that describes the client's user agent
		std::string   media;    // Header that specifies which media types the client can accept
		std::string   connection;    // Header that specifies if we have to close the connection or keeping it alive

		// BODY
		std::string   body;
		std::string   content_type;
		size_t        lenght;
		
		Request();
		
	public :
		Request(char const *buffer, int read, int socket);
		Request(const Request & orig);
		~Request();
		Request &operator=(Request const & rhs);

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		std::string getSave_buffer() const; //A EFFACER
		
		int getStatus() const;
		int getSocket_fd() const;
		std::string getHost() const;
		std::string getConnection() const;
		void	addSave_buffer(const char *buffer);
		void	setStatus(int status);
		
        /* ***************************************************************** */
        /* **************************** Parsing **************************** */
        /* ***************************************************************** */

		void	parse_request();
		
        /* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
		
        int  handle_request(int socket_fd, t_conf &conf, ErrorPages &error);
		void	build_response(int socket_fd, t_conf &conf, ErrorPages &error);
		void	send_response(int socket_fd);

		/* ***************************************************************** */
		/* *************************** Location **************************** */
		/* ***************************************************************** */
		
		std::string look_for_location(t_conf & conf);
		std::string look_if_location(std::string &target, t_conf & conf);
		void	add_path(t_conf & conf, std::string &index);
		
		/* ***************************************************************** */
		/* ***************************** Method **************************** */
		/* ***************************************************************** */
		
		bool	check_allow_method(t_conf &conf, std::string &index);
		
		/* ***************************************************************** */
		/* ***************************** Utils ***************************** */
		/* ***************************************************************** */

		std::string extract_line(std::string & buff, char delim) const;
		std::string extract_header(std::string & buff) const;
		std::string extract_elem(std::string const &elem, std::string const &delim, std::string & buff) const;
} ;

#endif