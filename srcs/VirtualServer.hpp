#ifndef VIRTUALSERVER_HPP
#define VIRTUALSERVER_HPP

#include "./Header.hpp"
#include "Request.hpp"
#include "Client.hpp"

class VirtualServer
{
	public:
		VirtualServer(void){
			initAddr(AF_INET, INADDR_ANY, htons(PORT));
		}
		VirtualServer(std::vector<std::string> file){
			this->_conf = file;
			this->_root = "./public";
			this->parsing();
			this->verifications();
			initAddr(AF_INET, INADDR_ANY, htons(atoi(this->_listen.c_str())));
			initFd(AF_INET , SOCK_STREAM , 0);
			initLink();
			initListen(4);
		}
		VirtualServer(VirtualServer const &rhs){
			operator=(rhs);
		}
		virtual ~VirtualServer(void){}
		VirtualServer &														operator=( VirtualServer const &rhs){
			if (this != &rhs){
			}
			return (*this);
		}
		class IncorrectMethodUsed : public std::exception{
			public:
				IncorrectMethodUsed(void){}
				IncorrectMethodUsed( IncorrectMethodUsed const & src){this->operator=(src);}
				virtual ~IncorrectMethodUsed(void)throw(){}
				IncorrectMethodUsed &	operator=(IncorrectMethodUsed const & rhs){(void)rhs; return (*this);}
				virtual const char* what()const throw(){return ("Error : Incorrect Method Used.");}
		};

		/***************************************************
		*******************    Socket    *******************
		***************************************************/
		struct sockaddr_in													initAddr(int family, in_addr_t s_addr, in_port_t port){
			this->_address.sin_family = family;   
			this->_address.sin_addr.s_addr = s_addr;   
			this->_address.sin_port = port;
			return (this->_address); 
		}
		void																initLink(void){
			if (this->_fd != 0 && bind(this->_fd, (struct sockaddr *)&this->_address, sizeof(this->_address)) < 0){
				perror("bind failed");
				exit(EXIT_FAILURE);
			}
		}
		void																initListen(int number){
			if (listen(this->_fd, number) < 0){
				perror("listen");
				exit(EXIT_FAILURE);
			}
		}
		int																	initFd(int domain, int type, int protocol){
			int opt = TRUE;
			if( (this->_fd = socket(domain , type , protocol)) < 0){
				perror("socket failed");
				exit(EXIT_FAILURE);
			}
			if( setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){
				perror("setsockopt");
				exit(EXIT_FAILURE);
			}
			return (this->_fd);
		}

		/***************************************************
		********************    find    ********************
		***************************************************/
		size_t																findLocation(std::string uri){
			std::vector<size_t> index;

			for (size_t i = 0; i < this->_locations.size(); i++){
				if (this->_locations[i]["key"][0] == uri)
					return (i);
			}
			return (-1);
		}
		std::vector<size_t>													findLocationsAndSublocations(std::string uri){
			std::vector<size_t> index;

			while (uri.find('/') != SIZE_MAX){
				for (size_t i = 0; i < this->_locations.size(); i++){
					if (this->_locations[i]["key"][0] == uri)
						index.push_back(i);
				}
				uri = uri.erase(uri.size() - 1);
				uri = (uri.rfind('/') != SIZE_MAX) ? uri.substr(0,uri.rfind('/') + 1) : uri ;
			}
			return (index);
		}
		std::vector<std::string>											findOption(std::string option, std::string uri, std::vector<std::string> global){
			std::vector<size_t> indexs;
			std::vector<std::string> result;

			indexs = findLocationsAndSublocations(uri);
			for (size_t i = 0; i < indexs.size(); i++) {
				if (indexs[i] != SIZE_MAX && !this->_locations[indexs[i]][option].empty()) {
					result = this->_locations[indexs[i]][option];
					return (result);
				}
			}
			if (result.empty())
				for (size_t i = 0; i < global.size(); i++)
					result.push_back(global[i]);
			return (result);
		}
		std::string															findOption(std::string option, std::string uri, std::string global){
			size_t index;
			std::string result;

			index = findLocation(uri);
			if (index != SIZE_MAX && !this->_locations[index][option].empty()) //If we find the option, we split in vector
				result = this->_locations[index][option][0];
			if (result.empty())
				for (size_t i = 0; i < global.size(); i++)
					result = global;
			return (result);
		}
		
		/***************************************************
		***************    find variables    ***************
		***************************************************/
		bool																findAutoIndex(std::string path){
			std::string result;
			result = this->findOption("autoindex", path, this->get_autoIndex(1));
			return (result == "on");
		}
		std::vector<std::string>											findErrorPage(std::string path){
			std::vector<std::string> result;
			result = this->findOption("error_page", path, this->get_errorPage());
			return (result);
		}
		std::string															findErrorPage(std::string path, size_t number){
			return (findErrorPage(path)[number]);
		}
		std::string															findErrorPage(std::string path, std::string error){
			std::vector<std::string> vec = findMethod(path);
			for (size_t i = 0; i < vec.size(); i++){
				if (vec[i] == error){
					while (fileIsOpenable(findRoot(path) + vec[i]) == false)
						i++;
					if (fileIsOpenable(findRoot(path) + vec[i]))
						return (vec[i]);
				}
			}
			return ("");
		}
		std::string															findRoot(std::string path){
			std::string result;

			if (path.rfind('/') != path.size() - 1)
				path = path.substr(0, path.rfind('/') + 1);
			result = this->findOption("root", path, this->get_root());
			return (result);
		}
		std::vector<std::string>											findIndex(std::string path){
			std::vector<std::string> result;
			result = this->findOption("index", path, this->get_index());
			return (result);
		}
		std::string															findIndex(std::string path, size_t number){
			return (findIndex(path)[number]);
		}
		std::vector<std::string>											findMethod(std::string path){
			std::vector<std::string> result;
			result = this->findOption("method", path, this->get_method());
			return (result);
		}
		std::string															findMethod(std::string path, size_t number){
			return (findMethod(path)[number]);
		}
		bool																findMethod(std::string path, std::string method){
			std::vector<std::string> vec = findMethod(path);
			if (!vec.empty())
			{
				for (size_t i = 0; i < vec.size(); i++){
					if (vec[i] == method)
						return (true);
				}
				return (false);
			}
			return (true);
		}
		std::vector<std::string>											findCGI(std::string path, std::string extension){
			std::vector<std::string> result;
			if (extension != ""){
				result = this->findOption(extension, path, this->get_CGI()[extension]);
			}
			return (result);
		}
		std::string															findCGI(std::string path, std::string extension, std::string methode){
			std::vector<std::string> result;
			if (extension != ""){
				result = this->findOption(extension, path, this->get_CGI()[extension]);
				if (!result.empty()){
					for (size_t i = 0; i < result.size(); i++){
						if (result[i] == methode || result.size() == 1 || methode.empty())
							return (result[0]);
					}
					return ("bad_method");
				}
			}
			return ("no_cgi");
		}

		/***************************************************
		********************    GET   **********************
		***************************************************/
		struct sockaddr_in *												get_address(void){
			return (&this->_address);
		}
		bool																get_autoIndex(void){
			return (this->_autoIndex);
		}
		std::string															get_autoIndex(int i){
			if (i){
				if (this->_autoIndex)
					return ("on");
				return ("off");
			}
			return ("off");
		}
		std::map<std::string, std::vector<std::string> >					get_CGI(void){
			return (this->_CGI);
		}
		std::vector<std::string>											get_CGI(std::string extension){
			return (this->_CGI[extension]);
		}
		std::string															get_CGI(std::string extension, std::string methode){
			std::vector<std::string> result = this->_CGI[extension];
			for (size_t i = 0; i < result.size(); i++){
				if (result[i] == methode)
					return (result[0]);
			}
			return ("");
		}
		std::vector<Client *>												get_clients(void){
			return (this->_clients);
		}
		std::vector<std::string>											get_errorPage(void){
			return (this->_errorPage);
		}
		std::string															get_errorPage(size_t i){
			return (this->_errorPage[i]);
		}
		int																	get_fd(void){
			return (this->_fd);
		}
		std::map<std::string, std::string>									get_history(void){
			return (this->_history);
		}
		std::string															get_history(std::string user){
			return (this->_history[user]);
		}
		std::vector<std::string>											get_index(void){
			return (this->_index);
		}
		std::string															get_index(size_t i){
			return(this->_index[i]);
		}
		std::string															get_listen(void){
			return (this->_listen);
		}
		std::vector<std::map<std::string, std::vector<std::string> > >		get_locations(void){
			return (this->_locations);
		}
		std::vector<std::string>											get_method(void){
			return (this->_methods);
		}
		std::string															get_root(void){
			return (this->_root);
		}
		std::string															get_serverNames(void){
			return (this->_serverNames);
		}
		Client *															get_client(int i){
			return (this->_clients[i]);
		}

		/***************************************************
		********************    SET   **********************
		***************************************************/
		void																setHistory(std::string user, std::string url){
			this->_history[user] = url;
		}
		void																setClient(Client *client){
			return (this->_clients.push_back(client));
		}

		/***************************************************
		********************    DEL   **********************
		***************************************************/
		void																delClient(Client* ptr){
			for (std::vector<Client *>::iterator it = this->_clients.begin(); it != this->_clients.end(); it++){
				if (*it == ptr){
					this->_clients.erase(it);
					return ;
				}
			}
		}
		/***************************************************
		******************    Parsing    *******************
		***************************************************/
		void																parsing(void){
			this->parsingListen();
			this->parsingServerNames();
			this->parsingRoot();
			this->parsingIndex();
			this->parsingLocations();
			this->parsingAutoIndex();
			this->parsingErrorPages();
			this->parsingMethods();
			this->parsingMaxBody();
			this->parsingCGI();
		}
		void																parsingAutoIndex(void){
			unsigned int cpt = 0;
			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_conf[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_conf[i].find("autoindex") != SIZE_MAX && (cpt == 1)){
					if (this->_conf[i].find("off") != SIZE_MAX)
						this->_autoIndex = false;
					else
						this->_autoIndex = true;
					return ;
				}
			}
		}
		void																parsingIndex(void){
			unsigned int cpt = 0;
			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_conf[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_conf[i].find("index") == 0 && (cpt == 1)){
					std::string iss = this->_conf[i];
					iss = convertInSpaces(iss);
					iss = cleanLine(iss);
					std::vector<std::string> results = split(iss, " ");
					results.erase(results.begin());
					this->_index = results;
				}
			}
		}
		void																parsingListen(void){
			unsigned int cpt = 0;
			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_conf[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_conf[i].find("listen") != SIZE_MAX && (cpt == 1)){
					std::vector<std::string> results = split(_conf[i], " ");
					results.erase(results.begin());
					this->_listen = results[0];
				}
			}
		}
		void																parsingLocations(void){
		std::map<std::string, std::vector<std::string> > value;
		for (unsigned int i = 0; i < this->_conf.size(); i++){
			if (this->_conf[i].find("location") != SIZE_MAX){
				std::string qss = this->_conf[i];
				qss = convertInSpaces(qss);
				qss = cleanLine(qss);
				std::vector<std::string> res = split(qss, " ");
				value["key"].push_back(res[1]);
				unsigned int j = (this->_conf[i].find("{") != SIZE_MAX) ? i + 1 : i + 2;
				while (value["key"][0].find_last_not_of(" \t") != value["key"][0].size() -1 && value["key"][0].find_first_not_of(" \t") != SIZE_MAX)
						value["key"][0].erase(value["key"][0].size() - 1);
				while (this->_conf[j].find("}") == SIZE_MAX && j < this->_conf.size()){
					std::string iss = this->_conf[j];
					iss = convertInSpaces(iss);
					iss = cleanLine(iss);
					std::vector<std::string> results = split(iss, " ");
					std::string key = results[0];
					results.erase(results.begin());
					if (key == "error_page"){
						for (size_t i = 0; i < results.size(); i++)
							value[key].push_back(results[i]);
					}
					else{
						if (key == "CGI"){
							key = results[0];
							results.erase(results.begin());
						}
						value[key] = results;
					}
					j++;
				}
				this->_locations.push_back(value);
				value.clear();
			}
		}
		}
		void 																parsingErrorPages(void){
			unsigned int cpt = 0;
			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_conf[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_conf[i].find("error_page") != SIZE_MAX && (cpt == 1)){
					std::vector<std::string> results = split(_conf[i], " ");
					results.erase(results.begin());
					for (size_t i = 0; i < results.size(); i++)
						this->_errorPage.push_back(results[i]);
				}
			}
		}
		void																parsingRoot(void){
			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("root") != SIZE_MAX){
					std::vector<std::string> results = split(_conf[i], " ");
					results.erase(results.begin());
					this->_root = results[0];
					return ;
				}
			}
		}
		void																parsingServerNames(void){
			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("server_name") != SIZE_MAX){
					std::string iss = this->_conf[i];
					iss = convertInSpaces(iss);
					iss = cleanLine(iss);
					std::vector<std::string> results = split(iss, " ");
					results.erase(results.begin());
					this->_serverNames = results[0];
				}
			}
		}
		void																parsingMaxBody(void){
			size_t cpt = 0;

			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_conf[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_conf[i].find("maxBody") != SIZE_MAX && (cpt == 1))
					this->_maxBody = split(this->_conf[i], " ")[1];
			}
		}
		void																parsingMethods(void){
			size_t cpt = 0;

			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_conf[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_conf[i].find("method") != SIZE_MAX && (cpt == 1)){
					this->_methods = split(this->_conf[i], " ");
					this->_methods.erase(this->_methods.begin());
				}
			}
		}
		void																parsingCGI(void){
			size_t cpt = 0;
			std::vector<std::string> result;
			std::string key;

			for (unsigned int i = 0; i < this->_conf.size(); i++){
				if (this->_conf[i].find("{") != SIZE_MAX)
					cpt++;
				else if (this->_conf[i].find("}") != SIZE_MAX)
					cpt--;
				if (this->_conf[i].find("CGI") != SIZE_MAX && (cpt == 1)){
					result = split(this->_conf[i], " ");
					result.erase(result.begin());
					key = result[0];
					result.erase(result.begin());
					this->_CGI[key] = result;
				}
			}
		}

		/***************************************************
		*******************    Verif    ********************
		***************************************************/
		void 																verifications(void){
			if (!verifAllPathsInLocations() || !verifAllMethods() || !verifListen()
			||  !verifGblErrorPages() || !verifLocationsErrorPages()|| !verifGblRoot() || !verifLocationRoot() || !verifServerName()
			|| !verifLocationsPaths())
				exit(-1);
		}
		bool																verifMethod(std::string method) {
			std::string validMethods[8] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"};
			for (size_t i = 0; i < 8; i++)
				if (method == validMethods[i])
					return (true);
			return (false);
		}
		bool																verifAllMethods(void) {
			for (size_t i = 0; i < this->_methods.size(); i++) {
				if (!verifMethod(this->_methods[i])) {
					std::cerr << RED << "Error: Bad configuration of the global methods : " << this->_methods[i] << RESET << std::endl;
					return (false);
				}
			}
			for (size_t i = 0; i < this->_locations.size(); i++) {
				if (!this->_locations[i]["method"].empty()) {
					if (!verifMethod(this->_locations[i]["method"][0])) { // A verifier
						std::cerr << RED << "Error: Bad configuration of method parameter in the location " << this->_locations[i]["key"][0] << RESET << std::endl;
						return (false);
					}
				}
			}
			return (true);
		}
		bool																verifListen(void){
			if (this->_listen.empty()){
				std::cerr << RED << "Error: No configuration for listen parameter" << RESET << std::endl;
				return (false);
			}
			for (size_t i = 0; i < this->_listen.size(); i++){
				if (atoi(this->_listen.c_str()) < 0 || atoi(this->_listen.c_str()) > 65535) {
					std::cerr << RED << "Error: Bad configuration in listen parameter" << RESET << std::endl;
					return (false);
				}
			}
			return (true);
		}
		bool																verifGblErrorPages(void){
			if (this->_errorPage.empty())
				return (true);
			for (size_t i = 0; i < this->_errorPage.size(); i++) {
				if ((atoi(this->_errorPage[i].c_str()) < 100 || atoi(this->_errorPage[i].c_str()) > 599)){
					if (this->_errorPage[i][0] != '/' && this->_errorPage[i][0] != '.'){
						std::cerr << RED << "Error: Bad configuration of ports in global errorPages parameter impossible to set port : " << this->_errorPage[i] << RESET << std::endl;
						return (false);
					}
					else{
						std::string path = this->_root + this->_errorPage[i];
						if (!fileIsOpenable(path)){
							std::cerr << RED << "Error: Bad configuration of path in global errorPages parameter : " << this->_errorPage[i] << RESET << std::endl;
							return (false);
						}
					}
				}
			}
			return (true);
		}
		bool																verifLocationsErrorPages(void){
			for (size_t i = 0; i < this->_locations.size(); i++) {
				if (!this->_locations[i]["error_page"].empty()) {
					for (size_t j = 0; j < this->_locations[i]["error_page"].size(); j++) {
						if (atoi(this->_locations[i]["error_page"][j].c_str()) < 100 || atoi(this->_locations[i]["error_page"][j].c_str()) > 599) {
							if (this->_locations[i]["error_page"][j][0] != '/' && this->_locations[i]["error_page"][j][0] != '.'){
								std::cerr << RED << "Error: Bad configuration of ports in global errorPages parameter impossible to set port : " << this->_locations[i]["error_page"][j] << RESET << std::endl;
								return (false);
							}
							else{
								std::string path = findRoot(this->_locations[i]["key"][0]) + this->_locations[i]["error_page"][j];
								if (!fileIsOpenable(path)){
									std::cerr << RED << "Error: Bad configuration of path in global errorPages parameter : " << this->_locations[i]["error_page"][j] << RESET << std::endl;
									return (false);
								}
							}
						}
					}
				}
			}
			return (true);
		}
		bool																verifGblIndex(void) {
			if (this->_index.empty())
				return (true);
			for (size_t i = 0; i < this->_index.size(); i++) {
				if (this->_index[i][0] != '/') {
					std::cerr << RED << "Error: Bad configuration of path in global index parameter" << RESET << std::endl;
					return (false);	
				}
			}
			return (true);
		}
		bool																verifLocationIndex(void) {
			for (size_t i = 0; i < this->_locations.size(); i++) {
				if (!this->_locations[i]["index"].empty()) {
					for (size_t j = 0; j < this->_locations[i]["index"].size(); j++) {
						if (this->_locations[i]["index"][j][0] != '/') {
							std::cerr << RED << "Error: Bad configuration of path in index parameter of location : " << _locations[i]["key"][0] << RESET << std::endl;
							return (false);	
						}
					}
				}
			}
			return (true);
		}
		bool																verifGblRoot(void){
			for (size_t i = 0; i < this->_root.size(); i++) {
				if (!folderIsOpenable(this->_root) || this->_root[this->_root.size() - 1] == '/' ||
				 (this->_root[0] != '/' && this->_root[0] != '.'))
				{
					std::cerr << RED << "Error: Bad configuration of path in global root parameter" << RESET << std::endl;
					return (false);	
				}
			}
			return (true);
		}
		bool																verifLocationRoot(void) {
			for (size_t i = 0; i < this->_locations.size(); i++) {
				if (!this->_locations[i]["root"].empty()) {
					for (size_t j = 0; j < this->_root.size(); j++) {
						if (!folderIsOpenable(this->_locations[i]["root"][0]) ||
						 (this->_locations[i]["root"][0][0] != '/' && this->_locations[i]["root"][0][0] != '.')) {
							std::cerr << RED << "Error: Bad configuration of path in root parameter of location" << this->_locations[i]["key"][0] << RESET << std::endl;
							return (false);	
						}
					}
				}
			}
			return (true);
		}
		bool																verifServerName(void) {
			if (this->_serverNames.empty()) {
				std::cerr << RED << "Error: Bad configuration of server_name parameter" << RESET << std::endl;
				return (false);
			}
			return (true);
		}
		bool																verifLocationsPaths(void) {
			for (size_t i = 0; i < this->_locations.size(); i++) {
				if (this->_locations[i]["key"].empty()) {
					std::cerr << RED << "Error: Path of location " << i + 1 << " is empty" << RESET << std::endl;
					return (false);
				}
			}
			return (true);
		}
		bool																verifAllPathsInLocations(void) {
			for (size_t i = 0; i < this->_locations.size(); i++) {
				std::map<std::string, std::vector<std::string> >::iterator it;
				for (it = this->_locations[i].begin(); it != this->_locations[i].end(); it++) {
					for (size_t i = 0; i < it->second.size(); i++) {
						if (it->first != "key" && it->second[i][it->second[i].size() - 1] == '/') {
							std::cerr << RED << "Error: Bad initialisation of path in a location at : " << it->second[i] << RESET << std::endl;
							return (false);
						}
						else if (it->first == "key" && (it->second[0][0] != '/' || it->second[0][it->second[0].size() - 1] != '/')){
							std::cerr << RED << "Error: Bad initialisation of location path at : " << it->second[i] << RESET << std::endl;
							return (false);

						}
					}
				}
			}
			return (true);
		}

	private:
		struct sockaddr_in 													_address;
		bool																_autoIndex;
		std::map<std::string, std::vector<std::string> >					_CGI;
		std::vector<std::string> 											_conf;
		std::vector<std::string>											_errorPage;
		int 																_fd;
		std::map<std::string, std::string>									_history;
		std::vector<std::string>											_index;
		std::string															_listen;
		std::vector<std::map<std::string, std::vector<std::string> > >		_locations;
		std::string															_maxBody;
		std::vector<std::string>											_methods;
		std::string															_root;
		std::string															_serverNames;
		std::vector<Client *>												_clients;
};

#endif