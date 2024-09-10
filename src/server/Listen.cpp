/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: garance <garance@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/28 15:43:55 by galambey          #+#    #+#             */
/*   Updated: 2024/09/05 15:15:03 by garance          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/* ************************************************************************* */
/* ************************ Constructor & Destructor *********************** */
/* ************************************************************************* */

Listen::Listen() : _fd(0), _port(0), _s_addr(0), _host("") {}

Listen::Listen(const Listen & orig) : _fd(orig._fd), _i_conf(orig._i_conf), _port(orig._port), _s_addr(orig._s_addr), _host(orig._host) {

	*this = orig; 
}

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

Listen &Listen::operator=(Listen const & rhs) {
	this->_index = rhs._index;
	return (*this); 
}

/* ************************************************************************* */
/* ******************************** Accessor ******************************* */
/* ************************************************************************* */

int	Listen::getFd() const {
	return (_fd);
}

int	Listen::getIndex() const {
	return (_index);
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

void	Listen::addIconf(int const i) {
	_i_conf.push_back(i);
}

void	Listen::setIndex(int const i) {
	_index = i;
}

/* ************************************************************************* */
/* ********************************* Close ********************************* */
/* ************************************************************************* */


void	Listen::close_fd() {
	close(_fd);
}
