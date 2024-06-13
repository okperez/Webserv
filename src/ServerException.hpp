/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 13:59:03 by galambey          #+#    #+#             */
/*   Updated: 2024/06/13 14:31:04 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVEREXCEPTION
# define SERVEREXCEPTION

# include <exception>

class ServerException : public std::exception {
	private :
		const char *_err;
		
	public:
		ServerException(const char *err);
		const char *what() const throw();
} ;

#endif