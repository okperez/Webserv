/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 17:50:17 by galambey          #+#    #+#             */
/*   Updated: 2024/09/05 15:48:19 by garance          ###   ########.fr       */
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
		std::vector<Listen> 			_server_fd; 	// Vector of server's sockets(reading on gateways)
		std::vector<Request> 			_requests;  	// Vector with all pending requests
		ErrorPages 						_error;			// Object with http codes and handling the error pages and redirections
		bool							_time_sigint;	// Time out for CGI
		Media							_auth_media;	// Object with Allowed Content types in the server
		struct pollfd 					*_fds;			// Array of struct containing the sockets that poll use manage I/O multiplexing
		bool 							_ctrlC;			// = true if SigInt happened
		
		Server(const Server & orig);
		Server &operator=(Server & rhs);
		
		/* ***************************************************************** */
		/* ************************* BFR LAUNCHING ************************* */
		/* ***************************************************************** */
		
		bool			check_port_binding(std::string const &port, std::string const &host, int const i);
		void			bind_socket(int const new_socket, struct sockaddr_in &server_addr, int const port, struct addrinfo *res);
		void			listen_socket(int const new_socket, int const port, struct addrinfo *res);
		struct addrinfo	*get_addr_info(const char *data, int const new_socket);
		
				/* ***************************************************************** */
		/* **************************** EVENTS ***************************** */
		/* ***************************************************************** */

		void			read_request(int const i, char const *buffer, int const read);
		bool			request_response(int const i);
		void			no_event_request(size_t const max_socket);
		void			event_request(size_t const max_socket);
		
		/* ***************************************************************** */
		/* *********************** HANDLE CONNECTION *********************** */
		/* ***************************************************************** */

		void			new_connection(size_t const max_socket);
		bool			new_connection(int const server_fd, int const max_socket);
		void			close_connection(int const i);

		/* ***************************************************************** */
		/* **************************** REQUEST **************************** */
		/* ***************************************************************** */
		
		int				unique_match(std::string const &port, std::string const &socket_ip, std::vector<int> & tmp);
		int				is_host(std::string const &host, std::string const &port, std::string const &socket_host);
		int				pick_server(Request &request);
		void			body_request_present(Request &request, int read, int i);

		/* ***************************************************************** */
		/* ***************************** ERROR ***************************** */
		/* ***************************************************************** */

		void			send_error(std::vector<Request>::iterator &it, std::string const &code, const char *mess, ErrorPages &error);
		void			handle_error_function(int const i);

		/* ***************************************************************** */
		/* ***************************** CLOSE ***************************** */
		/* ***************************************************************** */	
		
		void			error_bfr_launch(int const new_socket, struct addrinfo *res, const char *s);
		void			close_and_erase(std::vector<Request>::iterator &it);
		void			stop_listen();
		void			handle_pending_requests_in_vect(int const &socket);
		void			handle_pending_requests();

	public :
		std::vector<t_conf>	conf; 	// Vector containing the differents servers configuration
		
		Server();
		~Server();

		/* ***************************************************************** */
		/* **************************** Accessor *************************** */
		/* ***************************************************************** */

		struct pollfd 	*getFds() const;
		size_t			getServer_fd_size() const;
		void			setCtrlC();
		bool			getTimeSigint() const;
		void			setTimeSigint();

		/* ***************************************************************** */
		/* ************************* BFR LAUNCHING ************************* */
		/* ***************************************************************** */
		
		void			open_listen_socket();
		void			create_fds();
		
		/* ***************************************************************** */
		/* *************************** LAUNCHING *************************** */
		/* ***************************************************************** */

		void			launch_server(int const max_socket);
		
		/* ***************************************************************** */
		/* ***************************** CLOSE ***************************** */
		/* ***************************************************************** */	
		
		void			del_all();
		void			error_bfr_launch(); // POUR MAIN UNNIQUEMENT
		void			close_child_sockets();

} ;

#endif