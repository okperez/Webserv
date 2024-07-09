/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NotAnIntException.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 14:29:18 by galambey          #+#    #+#             */
/*   Updated: 2024/07/08 18:38:37 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

const char *NotAnIntException::what() const throw() {
	return ("Not an int");
}
