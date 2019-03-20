#include <stdlib.h>
#include <iostream>
#include "HttpdServer.hpp"

static void Usage(std::string proc_){
	std::cout<<"Usage:"<<proc_<<" port_number"<<std::endl;
}

int main(int argc,char* argv[]){

	if(argc != 2){
		Usage(argv[0]);
		exit(1);
	}
	
	//创建服务器实体
	HttpdServer* serverp_ = new HttpdServer(atoi(argv[1]));
	//初始化服务器
	serverp_->InitServer();
	//启动服务器
	serverp_->Start();

	delete serverp_;
	return 0;

}
