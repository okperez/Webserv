/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Media.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/09/04 13:58:18 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MEDIA_HPP
# define MEDIA_HPP

#include "include.hpp"

typedef struct s_conf t_conf;

class Media
{
	private : 
		std::map<std::string, std::vector<std::string> > _types;
	
		Media(const Media & orig);
		Media &operator=(Media & rhs);
		
	public :
		Media();
		~Media();
		std::map<std::string, std::vector<std::string> >	& getTypes();

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