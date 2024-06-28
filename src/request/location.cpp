/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: galambey <galambey@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/19 11:52:24 by galambey          #+#    #+#             */
/*   Updated: 2024/06/28 17:50:01 by galambey         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

/*
Recherche pour la location de la plus precise a la moins precise dans le serveur
*/
// PENSER A TESTER SI / TOUJOURS AVEC CURL DANS REQUETE COMME POUR HTTP
std::string look_if_location(std::string &target, t_conf & conf) {

	// std::cout << "Look_for_location avec target = " << target << std::endl;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(target);
	if (it != conf.location.end()) {
		// std::cout << "FIND : " << it->first << std::endl;
		return (it->first);	
	}
	std::string s = target;
	if (target.back() == '/')
		s.erase(s.length() - 1, 1); 
	else {
		size_t found = s.rfind('/');
		if (found == std::string::npos) {
			// std::cout << "NOT FOUND" << std::endl; // si not found + pas de root globale pour le serveur => page d erreur en dur 
			return ("");
		}
		s.erase(found + 1, s.length() - found + 1);
	}
	return (look_if_location(s, conf));
}

/*
Cherche si une location = correspondante a la target existe, sinon appelle look_if_location
*/
// A FAIRE : VOIR COMMENT GERER =/loc et =/loc/index.html
std::string look_for_location(std::string &target, t_conf & conf) {
	
	std::string s = "=" + target;
	std::map< std::string, std::map<std::string, std::string> >::iterator it;
	it = conf.location.find(s);
	if (it != conf.location.end())
		return (it->first);
	// std::cout << "hey" << std::endl;
	return (look_if_location(target, conf));
	//on recupere l index de la location :)
}

/* Supprime la location et la remplace par la root */
void	add_path(std::string &target, t_conf & conf, std::string &index) {
	
	std::string root;
	
	if (conf.location[index].find("root") == conf.location[index].end())
		root = conf.root_dir;
	else
		root  = conf.location[index]["root"];
	if(index.back() == '/' && root.back() != '/')
		root += "/";
	if (index.front() == '=')
		index.erase(0, 1);
	target.replace(target.find(index), index.length(), root);
	// std::cout << "target : " << target << std::endl;
}