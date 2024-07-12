/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Media.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/07/12 17:38:40 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MEDIA_HPP
# define MEDIA_HPP

#include "include.hpp"

typedef struct s_conf t_conf;

class Media
{
	private : 
		std::map<std::string, std::vector<std::string> > types;
	
		Media(const Media & orig);
		Media &operator=(Media & rhs);
		
	public :
		Media();
		~Media();

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

        /* ***************************************************************** */
        /* **************************** Actions **************************** */
        /* ***************************************************************** */
        
		bool	is_allow(std::string const &index, std::string const &content);
		std::string	match_type(std::string const &type);
		
} ;

#endif