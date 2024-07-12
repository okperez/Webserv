/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 13:46:10 by galambey          #+#    #+#             */
/*   Updated: 2024/07/12 17:18:00 by galambey         ###   ########.fr       */
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
		std::string 						_body;
		std::string							_cookie;
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

		void	setAuthmedia(Media *auth_media);
		void	setStatus(std::string const & code, std::string const & tittle);
		bool	setContent_type(std::string const & s);
		void	setContent_length();
		void	setLocation(std::string const & s);
		void	reinitBody();
		void	setBody(std::string const & s);
		void	setBody(std::ifstream &file);

		std::string build_response();

		//A EFFACER
		void print();
		
} ;

#endif