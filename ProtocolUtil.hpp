#ifndef __PROTOCOL_UTIL_HPP__
#define __PROTOCOL_UTIL_HPP__

#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <unordered_map>
#include <sstream>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include"Log.hpp"

#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define PAGE_404 "404.html"

#define OK 200
#define NOT_FOUND 404
#define BAD_REQUEST 400
#define SERVER_ERROR 500

#define HTTP_VERSION "http/1.0"

std::unordered_map<std::string,std::string> suffix_map{
	{".html","text/html"},
	{".htm","text/html"},
	{".css","text/css"},
	{".js","application/x-javascript"}
};

class ProtocolUtil{
public:
	static void MakeKV(std::unordered_map<std::string,std::string>& kv_,std::string& str_){
		std::size_t pos_ = str_.find(": ");
		if(std::string::npos == pos_)
			return;
		
		std::string k_ = str_.substr(0,pos_);
		std::string v_ = str_.substr(pos_+2);

		kv_.insert(make_pair(k_,v_));
	}

	static std::string IntToString(int code_){
		std::stringstream ss;
		ss << code_;

		return ss.str();
	}

	static std::string CodeToDesc(int code_){
		switch(code_){
			case 200:
				return "OK";
			case 400:
				return "Bad Request";
			case 404:
				return "Not Found";
			case 500:
				return "Internal Server Error";
			default:
				return "Unknow";
		}
	}

	static std::string SuffixToType(const std::string& suffix_){
		return suffix_map[suffix_];
	}

};

class Request{
public:
	std::string rq_line;
	std::string rq_head;
	std::string blank;
	std::string rq_text;
private:
	std::string method;
	std::string uri;
	std::string version;
	
	bool cgi;

	std::string path;
	std::string param;

	int resource_size;
	std::string resource_suffix;

	std::unordered_map<std::string,std::string> head_kv;
	int content_length;
public:
	Request():blank("\n"),cgi(false),path(WEB_ROOT),resource_size(0),resource_suffix(".html"),content_length(0)
	{}

	void RequestLineParse(){
		std::stringstream ss(rq_line);
		ss >> method >> uri >> version;
	}

	bool IsMethodLegal(){
		if(strcasecmp(method.c_str(),"GET") == 0 || 
		  (cgi = strcasecmp(method.c_str(),"POST") == 0))
		    return true;
		else
		    return false;
	}

	void UriParse(){
		//忽略大小写比较
		if(strcasecmp(method.c_str(),"GET") == 0)
		{
		    std::size_t pos_ = uri.find('?');
		    if(std::string::npos != pos_)
		    {
		        cgi = true;
			path += uri.substr(0,pos_);
			std::cout<< "In UriParse:"  << path <<std::endl;
			param += uri.substr(pos_+1);
		    }
		    else
			path += uri;
		}
		else
		    path += uri;
		
		if(path[path.size()-1] == '/'){
			path += HOME_PAGE;
			std::cout<< "path is "<< path <<std::endl;
		}
	}

	bool IsPathLegal(){
		struct stat st;
		if(stat(path.c_str(),&st) < 0)
		{
		    LOG(WARNING,"path not found!");
		    return false;
		}
		
		if(S_ISDIR(st.st_mode))
		{
		    path += '/';
		    path += HOME_PAGE;
		}
		else
		{
		    if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
			cgi = true;
		}

		resource_size = st.st_size;
		std::size_t pos = path.rfind(".");
		if(std::string::npos != pos){
			resource_suffix = path.substr(pos);
		}

		return true;
	}

	bool RequestHeadParse(){
		int start_ = 0;
		while(start_ < rq_head.size()){

			std::size_t pos_ = rq_head.find("\n",start_);
			if(std::string::npos == pos_)
				return false;
			
			std::string sub_string_ = rq_head.substr(start_,pos_-start_);
			if(!sub_string_.empty())
			{
			    LOG(INFO,"sub_string is not empty!");
			    std::cout<< sub_string_ <<std::endl;
			    ProtocolUtil::MakeKV(head_kv,sub_string_);
			}
			else
			{
			    LOG(INFO,"sub_string is empty!");
			    return false;
			}

			start_ = pos_ + 1;
		}
		return true;
	}

	bool IsNeedRecvText(){
		if(strcasecmp(method.c_str(),"POST") == 0)
			return true;
		else
			return false;
	}

	int GetContentLength(){
		std::string s_ = head_kv["Content-Length"];
		if(!s_.empty())
		{
		   std::stringstream ss(s_);
		   ss >> content_length;
		}

		return content_length;
	}

	std::string& GetParam(){
		return param;
	}

	int GetResourceSize(){
		return resource_size;
	}

	std::string GetSuffix(){
		return resource_suffix;
	}

	std::string& GetPath(){
		return path;
	}

	void SetResourceSize(int size_){
		resource_size = size_;
	}

	bool IsCgi(){
		return cgi;
	}

	void SetPath(std::string path_){
		path = path_;
	}

	void SetSuffix(std::string suffix_){
		resource_suffix = suffix_;
	}
	
	~Request()
	{}
};

class Response{
public:
	std::string rsp_line;
	std::string rsp_head;
	std::string blank;
	std::string rsp_text;

	int fd;
	int code;
public:
	Response():blank("\n"),fd(-1),code(OK)
	{}

	void MakeStatusLine(){
		rsp_line += HTTP_VERSION;
		rsp_line += " ";
		rsp_line += ProtocolUtil::IntToString(code);
		rsp_line += " ";
		rsp_line += ProtocolUtil::CodeToDesc(code);
		rsp_line += "\n";
	}

	void MakeResponseHead(Request* &rq_){
		rsp_head = "Content-Length: ";
		rsp_head += ProtocolUtil::IntToString(rq_->GetResourceSize());
		rsp_head += '\n';
		rsp_head += "Content-Type: ";
		std::string suffix_ = rq_->GetSuffix();
		rsp_head += ProtocolUtil::SuffixToType(suffix_);
		rsp_head += '\n';
	}

	void OpenResource(Request* &rq_){
		std::string path_ = rq_->GetPath();
		fd = open(path_.c_str(),O_RDONLY);
	}

	~Response()
	{}
};

class Connect{
private:
	int sock;
public:
	Connect(int sock_):sock(sock_)
	{}

	int RecvOneLine(std::string &line_){
		char c_ = 'Y';
		while(c_ != '\n'){
		      ssize_t s = recv(sock,&c_,1,0);
		      if(s > 0)
		      {
		          if(c_ == '\r')
			  {
			      recv(sock,&c_,1,MSG_PEEK);
			      if(c_ == '\n')
			          recv(sock,&c_,1,0);
			      else
			          c_ = '\n';
			  }

			  line_.push_back(c_);
		       }
		       else
		          break;
		}
		return line_.size();
	}

	void RecvRequestHead(std::string& head_){
		head_ = "";
		std::string line_;
		while(line_ != "\n"){
			line_ = "";
			RecvOneLine(line_);

			if(line_ != "\n"){
				head_ += line_;
			}
		}
	}

	void RecvRequestText(std::string& text_,int len_,std::string& param_){
		char c_;
		for(int i = 0;i < len_;++i){
		    recv(sock,&c_,1,0);
		    text_.push_back(c_);
		}

		param_ = text_;
	}

	void SendResponse(Request* &rq_,Response* &rsp_,bool cgi_){
		std::string &rsp_line_ = rsp_->rsp_line;
		std::string &rsp_head_ = rsp_->rsp_head;
		std::string &blank_ = rsp_->blank;

		send(sock,rsp_line_.c_str(),rsp_line_.size(),0);
		send(sock,rsp_head_.c_str(),rsp_head_.size(),0);
		send(sock,blank_.c_str(),blank_.size(),0);

		if(cgi_)
		{
		    std::string& rsp_text_ = rsp_->rsp_text;
		    send(sock,rsp_text_.c_str(),rsp_text_.size(),0);
		}
		else
		{
		    int fd_ = rsp_->fd;
		    sendfile(sock,fd_,NULL,rq_->GetResourceSize());
		}
	}

	~Connect(){
		if(sock >= 0){
			close(sock);
		}
	}
};

class Entry{
public:

	static void ProcessNonCgi(Connect* &conn_,Request* &rq_,Response* &rsp_){
		rsp_->MakeStatusLine();
		rsp_->MakeResponseHead(rq_);
		rsp_->OpenResource(rq_);
		conn_->SendResponse(rq_,rsp_,false);
	}

	static void ProcessCgi(Connect* &conn_,Request* &rq_,Response* &rsp_){
		int &code_ = rsp_->code;

		//参数：get和post方法都可以传参
		std::string& param_ = rq_->GetParam();
		//响应正文
		std::string& rsp_text_ = rsp_->rsp_text;

		int input[2];
		int output[2];
		
		//input:作为从父进程到子进程的输入管道
		//output:作为从子进程到父进程的输出管道
		pipe(input);
		pipe(output);

		pid_t pid_ = fork();
		if(pid_ < 0)
		{
			code_ = SERVER_ERROR;
			return;
		}
		else if(pid_ == 0)
		{
			close(input[1]);
			close(output[0]);

			//请求资源（要执行的程序）的路径
			std::string& path_ = rq_->GetPath();
			
			//设置一个环境变量，这会让替换后的进程清楚的知道参数的大小，方便接受参数
			std::string cl_env_ = "Content-Length=";
			cl_env_ += ProtocolUtil::IntToString(param_.size());
			putenv((char*)cl_env_.c_str());

			//文件重定向
			dup2(input[0],0);
			dup2(output[1],1);
				
			//程序替换
			//std::cout<< "start execl" <<std::endl;
			execl(path_.c_str(),path_.c_str(),NULL);
			exit(1);
		}
		else
		{
			close(input[0]);
			close(output[1]);

			//将参数传到子进程
			size_t size_ = param_.size();
			size_t total_ = 0;
			size_t cuur_ = 0;
			const char* p_ = param_.c_str();
			std::cout<< "in cgi param is: " << param_ <<std::endl; 
			while(total_ < size_ && (cuur_ = write(input[1],p_ + total_,size_ - total_)) > 0){
				total_ += cuur_;
			}

			//从管道里读取响应正文
			char c_;
			while(read(output[0],&c_,1) > 0){
				rsp_text_.push_back(c_);
			}
			std::cout<< "in cgi rsp_text is: " << rsp_text_ <<std::endl;

			waitpid(pid_,NULL,0);

			close(input[1]);
			close(output[0]);

			rsp_->MakeStatusLine();
			rq_->SetResourceSize(rsp_text_.size());
			rsp_->MakeResponseHead(rq_);

			conn_->SendResponse(rq_,rsp_,true);
		}
	}

	static void ProcessResponse(Connect* &conn_,Request* &rq_,Response* &rsp_){
		if(rq_->IsCgi())
		    ProcessCgi(conn_,rq_,rsp_);
		else
		    ProcessNonCgi(conn_,rq_,rsp_);
	}	

	static void Process_404(Connect* &conn_,Request* &rq_,Response* &rsp_){
		std::string path_ = WEB_ROOT;
		path_ += '/';
		path_ += PAGE_404;
		
		struct stat st;
		stat(path_.c_str(),&st);

		rq_->SetResourceSize(st.st_size);
		rq_->SetSuffix(".html");
		rq_->SetPath(path_);

		ProcessNonCgi(conn_,rq_,rsp_);
	}

	static void HandlerError(Connect* &conn_,Request* &rq_,Response* &rsp_){
		int code_ = rsp_->code;
		switch(code_){
			case 400:
			     break;
			case 404:
			     Process_404(conn_,rq_,rsp_);
			     break;
			case 500:
			     break;
			case 503:
			     break;
		}
	}
	
	//处理请求函数
	static int HandlerRequest(int sock_){
		
		Connect* conn_ = new Connect(sock_);
		Request* rq_ = new Request();
		Response* rsp_ = new Response();
		int& code_ = rsp_->code;

		conn_->RecvOneLine(rq_->rq_line);
		rq_->RequestLineParse();
		if(!rq_->IsMethodLegal())
		{
		    conn_->RecvRequestHead(rq_->rq_head);
		    code_ = BAD_REQUEST;
		    goto end;
		}

		rq_->UriParse();
		if(!rq_->IsPathLegal())
		{
		    conn_->RecvRequestHead(rq_->rq_head);
		    code_ = NOT_FOUND;
		    goto end;
		}

		LOG(INFO,"request path is ok!");

		conn_->RecvRequestHead(rq_->rq_head);
		if(!rq_->RequestHeadParse())
		{
		    code_ = BAD_REQUEST;
		    goto end; 
		}
		
		LOG(INFO,"request head is ok!");

		if(rq_->IsNeedRecvText()){
			conn_->RecvRequestText(rq_->rq_text,rq_->GetContentLength(),rq_->GetParam());
		}

		ProcessResponse(conn_,rq_,rsp_);
		
		LOG(INFO,"response end");
	end:
		if(code_ != OK)
			HandlerError(conn_,rq_,rsp_);

		delete conn_;
		delete rq_;
		delete rsp_;
		return code_;

	}
};

#endif //__PROTOCOL_UTIL_HPP__
