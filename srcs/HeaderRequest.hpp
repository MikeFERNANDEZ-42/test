#ifndef HEADERREQUEST_HPP
#define HEADERREQUEST_HPP

#include "Header.hpp"
#include "Request.hpp"

class HeaderRequest {
	public:
		HeaderRequest() {
			this->addContent("HTTP/1.1", "200 OK");
		}
		virtual	~HeaderRequest() {}

		/***************************************************
		*****************    Operations    *****************
		***************************************************/
		void											addContent(std::string key, std::string content){
			this->_content.insert(std::pair<std::string, std::string>(key, (content + "\r\n")));
			this->_size += 1;
		}
		void											updateContent(std::string key, std::string content){
			if (this->_content.find(key) == this->_content.end()) {
				this->addContent(key, content);
				return ;
			}
			this->_content[key] = (content + "\r\n");
		}
		void											sendHeader(Request *req){
			std::string										rep;
			rep = "HTTP/1.1 " + this->_content["HTTP/1.1"];
			for (std::map<std::string, std::string>::iterator i = this->_content.begin(); i != this->_content.end(); i++) {
				if (i->first != "HTTP/1.1")
					rep += i->first + ": " + i->second;
			}
			rep.erase(rep.size() - 1);
			rep.erase(rep.size() - 1);
			rep += "\n\n";
			req->sendPacket(rep.c_str());
		}
		void											basicHeaderFormat(Request *req){
			this->addContent("Host", (req->get_host() + ":" + req->get_port()));
			this->updateContent("Content-Location", req->get_uri());
			this->addContent("Server", "webserv");
			this->addContent("Date", getTime());
			this->updateContent("Content-Type", "text/html");
			if (req->getMimeType(req->getExtension()) != "")
				this->updateContent("Content-Type", req->getMimeType(req->getExtension()));
			this->updateContent("Accept-Charset", "utf-8");
		}
		void											basicHistory(VirtualServer *vserv, Request *req){
			if (vserv->get_history((req->get_IpClient() + req->get_userAgent())) != "")
				this->updateContent("Referer", vserv->get_history((req->get_IpClient() + req->get_userAgent())));
			if (!folderIsOpenable((vserv->findRoot(req->get_uri()))))
				this->updateContent("Content-Length", NumberToString(getSizeFileBits(vserv->findRoot(req->get_uri()))));
		}
		void											Error405HeaderFormat(Request *req, std::string allowMethods){
			this->basicHeaderFormat(req);
			this->updateContent("HTTP/1.1", "405 Method Not Allowed");
			this->updateContent("Content-Type", "text/html");
			this->addContent("Allow", allowMethods);
		}
		void											RedirectionHeaderFormat(Request *req, std::string uri){
			this->basicHeaderFormat(req);
			this->updateContent("HTTP/1.1", "301 Moved Permanently");
			this->updateContent("Content-Type", "text/html");
			this->updateContent("Location", uri);
			this->updateContent("Retry-After", "1");
			this->updateContent("Connection", "keep-alive");
		}


		/***************************************************
		*********************    GET   *********************
		***************************************************/
		size_t											getSize(void) const{
			return (this->_content.size());
		}
		std::map<std::string, std::string>				getContent(void) const{
			return (this->_content);
		}
		std::string										getContent(std::string key){
			return (this->_content[key]);
		}

	private:
		std::map<std::string, std::string>				_content;
		size_t											_size;
};

#endif