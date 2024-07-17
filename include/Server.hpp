/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 17:50:17 by galambey          #+#    #+#             */
/*   Updated: 2024/07/17 11:09:14 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "include.hpp"
# include "ErrorPages.hpp"
# include "Media.hpp"
# include "Request.hpp"

typedef struct s_conf t_conf;

class Server
{
	private : 
		ErrorPages 	error;

		Server(const Server & orig);
		Server &operator=(Server & rhs);
		
		/* ***************************************************************** */
		/* ************************* BFR LAUNCHING ************************* */
		/* ***************************************************************** */
		
		struct addrinfo	*get_addr_info(char *data, int new_socket);
		bool	check_port_binding(std::vector<Listen> &server_fd, std::string &port, std::string &host, int i);
		// bool	check_port_binding(std::vector<Listen> &server_fd, std::string &port, std::string &host);
		void	bind_socket(int new_socket, struct sockaddr_in &server_addr, int port, struct addrinfo *res);
		// void	bind_socket(int new_socket, struct sockaddr_in &server_addr, int port);
		void	listen_socket(int new_socket, int port, struct addrinfo *res);
		
	public :
		Media		auth_media;
		std::vector<t_conf>	conf; // public si on n integre pas parsing dans classe
		std::vector<Listen> server_fd; // a passer en private une fois good
		std::vector<Request> requests; // a passer en private une fois good
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

		void	launch_server(int max_socket);
		void	event_request();
		/* ***************************************************************** */
		/* *********************** HANDLE CONNECTION *********************** */
		/* ***************************************************************** */

		void	new_connection(int server_fd);
		void	close_connection(int i);

		/* ***************************************************************** */
		/* **************************** REQUEST **************************** */
		/* ***************************************************************** */
		
		int		is_host(std::string host, std::string port, std::string socket_host);
		int		unique_match(std::string &port, std::string &socket_ip, std::vector<int> & tmp);
		int		pick_server(Request &request);
		void	body_request_present(Request &request, int read);
		void	read_request(int i, char *buffer, int read);

		/* ***************************************************************** */
		/* ***************************** ERROR ***************************** */
		/* ***************************************************************** */

		void	send_error(std::vector<Request>::iterator it, std::string const &code, const char *mess, ErrorPages &error);
		void	handle_error_function(int socket, std::string const &code, const char *mess, ErrorPages &error);

		/* ***************************************************************** */
		/* ***************************** CLOSE ***************************** */
		/* ***************************************************************** */	
		
		void	error_bfr_launch(int new_socket, struct addrinfo *res, const char *s);
		void	error_bfr_launch(); // POUR MAIN UNNIQUEMENT
		void	stop_listen();
		void	close_requests(int &socket);
		void	handle_pending_requests();
} ;

#endif