/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 13:46:10 by galambey          #+#    #+#             */
/*   Updated: 2024/09/05 16:02:01 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "include.hpp"
# include "Request.hpp"

class Response
{
	private : 
		std::string 						_status;
		std::string 						_content_type;
		std::string 						_content_length;
		std::string 						_location;
		std::string 						_connection;
		std::string 						_body;
		std::vector<std::string>			_cookie;
		Media								*_auth_media;
		
		Response(const Response &orig);
		
		bool		is_allowed_type(std::string const &index, std::string const  &content);
		
	public :
		Response();
		~Response();
		Response &operator=(Response const &rhs);

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		void		setAuthmedia(Media *auth_media);
		void		setStatus(std::string const &code, std::string const &tittle);
		void		setStatus(std::string const &code, ErrorPages &error);
		void		setContent_type(std::string const &s);
		void		setContent_length();
		void		setLocation(std::string const &s);
		void		setConnectiontoclose();
		void		reinitBody();
		size_t		setBody(std::string const &s);
		size_t		setBody(std::ifstream &file);
		void		setCookie(std::string const &str);

		/* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
        
		std::string build_response(size_t &body_len);
		void 		print_first_line(std::string const &response) const;
		
} ;

#endif