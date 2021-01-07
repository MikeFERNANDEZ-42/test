#ifndef ParsingRequest_HPP
#define ParsingRequest_HPP

#include "Header.hpp"

class ParsingRequest{
	public:
		ParsingRequest(){
			this->_extension = "";
			this->_file = "";
			this->_path = "";
		}
		virtual ~ParsingRequest(){}

		/***************************************************
		********************    GET   **********************
		***************************************************/
		std::string											getPath(void) const{
			return (this->_path);
		}
		std::string											getFile(void) const{
			return (this->_file);
		}
		std::string											getExtension(void) const{
			return (this->_extension);
		}
		std::map<std::string, std::string>					getMap(void) const{
			return (this->_map);
		}

		/***************************************************
		******************    PARSING   ********************
		***************************************************/
		void												parseGet(void){
			this->_path = "";

			this->_path = &this->_map["First"][this->_map["First"].find(" ") + 1];
			this->_path = (this->_path.find("?") != SIZE_MAX) ? this->_path.substr(0, this->_path.find("?") - 1)
			: this->_path.substr(0, this->_path.find("HTTP") - 1);
			this->_file = (this->_path.rfind("/") != SIZE_MAX) ? &this->_path[this->_path.rfind("/") + 1] : this->_path;
			this->_extension = (this->_file.rfind(".") != SIZE_MAX) ? &this->_file[this->_file.rfind(".")] : "";
		}
		void												parsingMime(void){
			std::ifstream			file("srcs/mime.types");
			std::string				line;

			while (getline(file, line)){
				size_t end = line.find_first_of("\t",0);
				std::string key = line.substr(0, end);
				size_t start = line.find_first_not_of("\t",end);
				std::list<std::string> value;
				while ((end = line.find_first_of(" ",start)) != SIZE_MAX){
					value.push_back(line.substr(start, end));
					start = end + 1;
				}
				this->_map_mime[key] = value;
			}
			file.close();
		}
		void												parsingMap(char *_buffer){

			std::vector<std::string> pars = split(_buffer, "\n");
			for (size_t i = 1; i < pars.size(); i++){
				if (pars[i] != "\n\r") // Condition a verifier
					pars[i] = cleanLine(pars[i]);
			}
			std::string key = "First";
			std::string value = (pars.size() > 0) ? pars[0] : "";
			this->_map[key] = value;
			
			for (size_t i = 1; i < pars.size(); i++){
				if (!pars[i].empty())
                	this->_map[pars[i].substr(0, pars[i].find_first_of(" ") - 1)] = &pars[i][pars[i].find_first_of(" ") + 1];
			}
		}

	private:
		std::string											_path;
		std::string											_file;
		std::string											_extension;
		std::map<std::string, std::string>					_map;
		std::map<std::string, std::list<std::string> >		_map_mime;
};

#endif