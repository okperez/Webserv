/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 17:50:17 by galambey          #+#    #+#             */
/*   Updated: 2024/06/28 18:19:04 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "include.hpp"

typedef struct s_conf t_conf;

class Server
{
	private : 
		Server(const Server & orig);
		Server &operator=(Server & rhs);
		
		/* ***************************************************************** */
		/* ************************* BFR LAUNCHING ************************* */
		/* ***************************************************************** */
		
		void	bind_socket(int new_socket, struct sockaddr_in &server_addr, int port);
		void	listen_socket(int new_socket, int port);
		
	public :
		std::vector<t_conf>	conf; // public si on n integre pas parsing dans classe
		std::vector<Listen> server_fd; // a passer en private une fois good
		struct pollfd *fds; // a passer en private une fois good
		
		Server();
		~Server(); // fds a free dans destructeur

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		/* ***************************************************************** */
		/* ************************* BFR LAUNCHING ************************* */
		/* ***************************************************************** */
		
		void	open_listen_socket();
		void	create_fds();
		
		/* ***************************************************************** */
		/* *************************** LAUNCHING *************************** */
		/* ***************************************************************** */

		
} ;

#endif