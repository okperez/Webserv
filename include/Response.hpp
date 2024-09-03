/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 13:46:10 by galambey          #+#    #+#             */
/*   Updated: 2024/09/03 17:54:02 by galambey         ###   ########.fr       */
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
		bool								_error;
		Media								*auth_media;
		
		Response(const Response & orig);
		
		bool	is_allowed_type(std::string const &index, std::string const  &content);
		
	public :
		Response();
		~Response();
		Response &operator=(Response const & rhs);

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		void						setAuthmedia(Media *auth_media);
		void						setStatus(std::string const & code, std::string const & tittle);
		void						setStatus(std::string const & code, ErrorPages &error);
		bool						setContent_type(std::string const & s);
		void						setContent_length();
		void						setLocation(std::string const & s);
		void						setConnectiontoclose();
		void						reinitBody();
		size_t						setBody(std::string const & s);
		size_t						setBody(std::ifstream &file);
		void						setCookie(std::string str);
		void						setError(bool err);
		bool						getError() ;

		std::string build_response(size_t *body_len);

		//A EFFACER
		void print();
		
} ;

#endif