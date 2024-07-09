/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/01 13:46:10 by galambey          #+#    #+#             */
/*   Updated: 2024/07/09 17:07:14 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "include.hpp"

class Response
{
	private : 
		std::string 	_status;
		std::string 	_content_type;
		std::string 	_content_length;
		std::string 	_location;
		std::string 	_body;
	
		Response(const Response & orig);
		
	public :
		Response();
		~Response();
		Response &operator=(Response const & rhs);

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		void	setStatus(std::string const & code, std::string const & tittle);
		void	setContent_type(std::string const & s);
		void	setContent_length();
		void	setLocation(std::string const & s);
		void	setBody(std::string const & s);
		void	setBody(std::ifstream &file);

		std::string build_response() const;

		//A EFFACER
		void print() const;
		
} ;

#endif