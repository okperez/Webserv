/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPages.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/07/22 10:29:22 by garance          ###   ########.fr       */
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
		std::map<std::string, std::string> map_error;
	
		ErrorPages(const ErrorPages & orig);
		ErrorPages &operator=(ErrorPages & rhs);
		
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
        
		void	err_not_found(Response &response, std::string &code);
		void	fill_significant_error(Response &response, std::string code, t_conf &conf);
        void	fill_error(Response &response, std::string code, t_conf &conf);
		void	fill_significant_error(Response &response, std::string code);
        void	fill_error(Response &response, std::string code);
		void	fill_redir(Response &response, std::string const &code, std::string const &redir);
        // void	err_not_found(std::string &body, std::string &code);
        // void	fill_error(std::string &body, std::string &response, std::string code, t_conf &conf);
} ;

#endif