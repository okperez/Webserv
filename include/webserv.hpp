/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: operez <operez@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 17:03:27 by operez            #+#    #+#             */
/*   Updated: 2024/09/10 11:34:34 by operez           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Listen.hpp"
# include "Server.hpp"
# include "ErrorPages.hpp"
# include "Request.hpp"
# include "ServerException.hpp"
# include "Media.hpp"

# define MAX_CONNECTION	255
# define BUFFER_SIZE	1000
# define DFLT_BODY_SIZE "100000"

typedef struct s_flag
{
    int loc; 
    int err_pgs;
    int ret;
    int ret_loc;
}t_flag;

typedef struct s_conf
{
  std::vector<std::string>                                  ipv4_port;
  std::vector<std::string>                                  ipv6_port;
  std::string												max_body_size;
  int														limit_body_size;
  std::string												host;
  std::string                                               server_name;
  std::string                                               root_dir;
  std::string                                               files;
  std::vector<std::string>                                  files_vect;
  std::string                                               autoindex;
  std::string                                               ret; // return
  std::map<std::string, std::map<std::string, std::string> >location;
  std::map<std::string, std::string>					    err_pgs;
  std::list<std::string>                                    wrong_arg;
  s_flag                                                    flag;
}t_conf;

class   ConfFileException : public std::exception
{
    public:
    
    char const * s;
    ConfFileException(char const* str) : s(str) {}
    virtual const char* what() const throw() // Attention def fonction dans .hpp
    {
        return (s);
    }
};

class	NotAnIntException : public std::exception {
			const char *what() const throw();	
};

/* *************************** CONFIGURATION_FILE *************************** */

int         handle_conf_file(char *argv, std::vector<t_conf> & conf);
void        check_bracket(std::list<std::string> & cnf_file);
void        check_syntax(std::list<std::string> & cnf_file);
void        set_conf_struct(std::list<std::string> & cnf_file, t_conf & conf);
std::string extract_conf(std::string buff, char c);
void        handle_locations(std::list<std::string> & cnf_file, t_conf & conf);
void        handle_err_pgs(std::list<std::string> & cnf_file, t_conf & conf);
void        compare_server(std::vector<t_conf> & conf, int  & server_nbr);
void        check_if_missing(t_conf & conf, std::list<std::string> & cnf_file);
void        check_if_valid(std::string str, int loc, int err_pgs);
void        handle_host(std::vector<t_conf> & conf);
void        check_returns(std::vector<t_conf> & conf);

/* ****************************** utils.cpp ******************************** */

void        clear_space(std::string & type);
bool        is_equal(t_conf & conf1, t_conf & conf2);
std::string clear_str(std::list<std::string> cnf_file);
bool        is_allow_methods(std::string str);
int         print_error(char const *str);
void        fill_map(std::map<int, std::string> & map, std::string str);
bool        is_white_space(char c, char d);
void        clear_space(std::string & type);
std::string extract_type(std::string buff);
std::string extract_index(std::string buff);
void	    str_tolower(std::string & s);
void	    setExtensions(std::map<std::string, char const *> &extensions);
void	    deleteArgs(char *const argv[], char *const envp[]);
void	    deleteArr(char *const env[], int index);

/* ********************************** UTILS ********************************* */
void	    strtomap(std::string s, std::map<std::string, std::vector<std::string> > & map, std::string const & delimstr, std::string const & delimmap);
int 		ft_stoi( std::string const & s );
void        strtovect(std::string s, std::vector<std::string> & v, std::string const & delim);
char		strback(std::string const & s);

/* ********************************* CTRL+C ********************************* */

void 			sighandler(int signal);
void            sighandlerbis(int signal);
void	        garbagge_server(Server *server, int rule);

/* ************************************************************************** */
/* ********************************** ENUM ********************************** */
/* ************************************************************************** */

enum	e_rule {
	PARENT,
	CHILDREN,
};

enum	e_status_request {
	NEW,
	READING,
	RD_TO_RESPOND,
	RD_TO_SEND,
	ERROR,
    CLOSE,
};

enum	e_method {
	GET,
	POST,
  	DELETE,
  	UNKNOWN,
};

#endif
