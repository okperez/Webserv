/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPages.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/09/04 14:21:54 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORPAGES_HPP
# define ERRORPAGES_HPP

#include "include.hpp"

typedef struct s_conf t_conf;
class Response;

class ErrorPages
{
	private : 
		std::map<std::string, std::string> _map_error;
	
		ErrorPages(const ErrorPages & orig);
		ErrorPages &operator=(ErrorPages & rhs);
		
		/* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
        
		void	err_not_found(Response &response, std::string const &code);
		
	public :
		ErrorPages();
		~ErrorPages();

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		std::string	get_message(std::string const & code);
		
        /* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
        
		void	fill_significant_error(Response &response, std::string const code, t_conf &conf);
        void	fill_error(Response &response, std::string const code, t_conf &conf);
		void	fill_significant_error(Response &response, std::string const code);
        void	fill_error(Response &response, std::string const code);
		void	fill_redir(Response &response, std::string const &code, std::string const &redir);
		void	del_all();
} ;

#endif