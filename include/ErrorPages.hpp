/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPages.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/06/29 11:26:43 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORPAGES_HPP
# define ERRORPAGES_HPP

#include "include.hpp"

typedef struct s_conf t_conf;

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

        /* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
        
        void	err_not_found(std::string &body, std::string &code);
        void	fill_error(std::string &body, std::string &response, std::string code, t_conf &conf);
} ;

#endif