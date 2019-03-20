#ifndef __HTTPD_SERVER_HPP__
#define __HTTPD_SERVER_HPP__

#include<pthread.h>
#include<arpa/inet.h>
#include"ProtocolUtil.hpp"
#include"ThreadPool.hpp"

class HttpdServer{
private:
	int listen_sock;
	int port;
	ThreadPool* tp;
public:
	HttpdServer(int port_):listen_sock(-1),port(port_),tp(NULL)
	{}	

	void InitServer(){

		listen_sock = socket(AF_INET,SOCK_STREAM,0);
		if(listen_sock < 0){
			LOG(ERROR,"creat socket error!");
			exit(2);
		}

		int opt_ = 1;
            	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt_, sizeof(opt_));

		struct sockaddr_in local_;
		local_.sin_family = AF_INET;
		local_.sin_port = htons(port);
		//INADDR:表示绑定任意地址
		local_.sin_addr.s_addr = INADDR_ANY;

		if(bind(listen_sock,(struct sockaddr*)&local_,sizeof(local_)) < 0){
			LOG(ERROR,"bind socket error!");
			exit(3);
		}

		if(listen(listen_sock,5) < 0){
			LOG(ERROR,"listen socket error!");
			exit(4);
		}
		
		//初始化服务器时new一个线程池，并对其进行初始化
		tp = new ThreadPool();
		tp->InitThreadPool();
		LOG(INFO,"InitServer success!");

	}

	void Start(){

		LOG(INFO,"Start server begin!");
		while(1){
			struct sockaddr_in peer_;
			socklen_t len_ = sizeof(peer_);
			int sock_ = accept(listen_sock,(struct sockaddr*)&peer_,&len_);
			if(sock_ < 0){
				LOG(WARNING,"accept error!");
				continue;
			}
			
			LOG(INFO,"accept new sock success");
			
			Task t;
			t.SetTask(sock_,Entry::HandlerRequest);
			tp->PushTask(t);
		}

	}
		
	~HttpdServer(){

		if(listen_sock != -1){
			close(listen_sock);
		}

		port = -1;

	}
};

#endif //__HTTPD_SERVER_HPP__

