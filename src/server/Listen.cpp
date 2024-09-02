/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:43:55 by galambey          #+#    #+#             */
/*   Updated: 2024/09/02 12:41:40 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Listen::Listen() : _fd(0), _port(0), _s_addr(0), _host("") {}

Listen::Listen(const Listen & orig) : _fd(orig._fd), _i_conf(orig._i_conf), _port(orig._port), _s_addr(orig._s_addr), _host(orig._host) { (void) orig; }

// Listen::Listen(int fd, std::string &port, int s_addr, std::string &host, int i) : _fd(fd), _port(port), _s_addr(s_addr), _host(host) , _i_conf(i) {}

Listen::Listen(int fd, std::string &port, int s_addr, std::string &host, int i) : _fd(fd), _port(port), _s_addr(s_addr), _host(host) {
	try {
		_i_conf.push_back(i);
	}
	catch (std::bad_alloc const & e) {
		close(_fd);
		throw ;
	}
	catch (std::length_error const & e) {
		close(_fd);
		throw ;
	}
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

int	Listen::getIndex() const {
	return (index);
}


std::vector<int>	Listen::getIconf() const {
	return (_i_conf);
}

std::string	Listen::getPort() const {
	return (_port);
}

in_addr_t	Listen::getS_addr() const {
	return (_s_addr);
}

std::string	Listen::getHost() const {
	return (_host);
}

void	Listen::addIconf(int i) {
	_i_conf.push_back(i);
}

void	Listen::setIndex(int const i) {
	index = i;
}

/* ************************************************************************* */
/* ******************************** Actions ******************************** */
/* ************************************************************************* */


void	Listen::close_fd() { // UTILISE?
	close(_fd);
}

void	Listen::printlisten() {
	std::cout << "FD : " << _fd << std::endl;
	for (std::vector<int>::iterator it= _i_conf.begin(); it != _i_conf.end(); it++)
		std::cout << "ICONF : " << *it << std::endl;
	std::cout << "PORT : " << _port << std::endl;
	std::cout << "S_ADDR : " << _s_addr << std::endl;
	std::cout << "HOST : " << _host << std::endl << std::endl;
}
