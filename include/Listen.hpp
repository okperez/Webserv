/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listen.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/06/28 17:35:10 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTEN_HPP
# define LISTEN_HPP

#include "include.hpp"

class Listen
{
	private : 
		const int			_fd;
		const int			_i_conf;
		const std::string	_port;
		const in_addr_t		_s_addr;
		const std::string 	_host;
	
		Listen();
		Listen &operator=(Listen & rhs);
		
	public :
		Listen(const Listen & orig);
		Listen(int fd, std::string &port, int _s_addr, std::string & host, int i);
		~Listen();

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		int	getFd() const;
		
		
		void	close_fd();
		
} ;

#endif