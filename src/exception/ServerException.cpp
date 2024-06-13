/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 14:29:18 by galambey          #+#    #+#             */
/*   Updated: 2024/06/13 17:35:21 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ServerException.hpp"

ServerException::ServerException(const char *err) : _err(err) {}

const char *ServerException::what() const throw() {
	return (_err);
}
