/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listen.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:39:47 by galambey          #+#    #+#             */
/*   Updated: 2024/07/09 10:01:44 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTEN_HPP
# define LISTEN_HPP

#include "include.hpp"

class Listen
{
	private : 
		const int			_fd; // socket_server
		// const int			_i_conf; // index du fichier conf => determiner quel serveur
		std::vector<int>			_i_conf; // index du fichier conf => determiner quel serveur
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
		std::vector<int>	getIconf() const;
		std::string	getPort() const;
		in_addr_t	getS_addr() const;
		std::string	getHost() const;
		
		void	addIconf(int i);
		
		void	close_fd();

		void	printlisten(); //A EFFACER
		
} ;

#endif