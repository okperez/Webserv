/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:43:55 by galambey          #+#    #+#             */
/*   Updated: 2024/07/04 09:59:17 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Listen::Listen() : _fd(0), _port(0), _s_addr(0), _host("") {}

Listen::Listen(const Listen & orig) : _fd(orig._fd), _port(orig._port), _s_addr(orig._s_addr), _host(orig._host), _i_conf(orig._i_conf) { (void) orig; }

// Listen::Listen(int fd, std::string &port, int s_addr, std::string &host, int i) : _fd(fd), _port(port), _s_addr(s_addr), _host(host) , _i_conf(i) {}

Listen::Listen(int fd, std::string &port, int s_addr, std::string &host, int i) : _fd(fd), _port(port), _s_addr(s_addr), _host(host) {
	_i_conf.push_back(i);
}

Listen::~Listen() {}

/* ************************************************************************* */
/* ************************** OPERATOR OVERLOADING ************************* */
/* ************************************************************************* */

/* ************************** Assignment Operator  ************************* */

Listen &Listen::operator=(Listen & rhs) {
	(void) rhs;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

int	Listen::getFd() const {
	return (_fd);
}

// int	Listen::getIconf() const {
// 	return (_i_conf);
// }

std::vector<int>	Listen::getIconf() const {
	return (_i_conf);
}

const std::string	Listen::getPort() const {
	return (_port);
}

const in_addr_t	Listen::getS_addr() const {
	return (_s_addr);
}

const std::string	Listen::getHost() const {
	return (_host);
}

void	Listen::addIconf(int i) {
	_i_conf.push_back(i);
}


/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */


void	Listen::close_fd() {
	close(_fd);
}

void	Listen::printlisten() {
	std::cout << "FD : " << _fd << std::endl;
	for (auto it= _i_conf.begin(); it != _i_conf.end(); it++)
		std::cout << "ICONF : " << *it << std::endl;
	std::cout << "PORT : " << _port << std::endl;
	std::cout << "S_ADDR : " << _s_addr << std::endl;
	std::cout << "HOST : " << _host << std::endl << std::endl;
}
