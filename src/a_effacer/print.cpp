// A EFFACER

#include "../../include/webserv.hpp"

void    print_request(t_request & request) {
    std::cout << "*********************" << std::endl;
    std::cout << "REQUEST AFTER PARSE" << std::endl;
    std::cout << "request.method = " << request.method << std::endl;
    std::cout << "request.target = " << request.target << std::endl;
    std::cout << "request.version = " << request.version << std::endl;
    std::cout << "request.host = " << request.host << std::endl;
    std::cout << "request.agent = " << request.agent << std::endl;
    std::cout << "request.media = " << request.media << std::endl;
    std::cout << "*********************" << std::endl;
}

void    print_location(std::vector<t_conf> &conf) {
    for (std::vector<t_conf>::iterator it = conf.begin(); it != conf.end(); it++) {
        std::cout << "          ************" << std::endl;
        std::cout << "server.name = " << it->server_name << std::endl;
        for (std::map<std::string, std::map<std::string, std::string> >::iterator itt = it->location.begin(); itt != it->location.end(); itt++) {
            std::cout << "  location index = " << itt->first << std::endl;
            for (std::map<std::string, std::string>::iterator ittt = itt->second.begin(); ittt != itt->second.end(); ittt++) {
                std::cout << "      " << ittt->first << " = " << ittt->second << std::endl;
            }
        }
    }
}