#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>

int main(){

	std::string param_;
	int size_ = atoi(getenv("Content-Length"));
	if(size_ > 0)
	{
	    std::cout<< "param size = " << size_ <<std::endl;
	    char c_;
	    while(size_){
		read(0,&c_,1);
		param_.push_back(c_);
		--size_;
	    }
	}

	std::cout<< "param is: " << param_ <<std::endl;	

	int a,b;
	sscanf(param_.c_str(),"a=%d&b=%d",&a,&b);
	std::cout << "<html>" << std::endl;
        //std::cout << "<head>My Cal</head>" <<std::endl;
        std::cout << "<body>" << std::endl;
        std::cout << "<h3>" << a << " + " << b << " = "<< a + b << "</h3>" <<std::endl;
        std::cout << "<h3>" << a << " - " << b << " = "<< a - b << "</h3>" <<std::endl;
        std::cout << "<h3>" << a << " * " << b << " = " << a * b << "</h3>" <<std::endl;
        if(b == 0){
            std::cout << "<h3>" << a << " / " << b << " = " << -1 << "(div zero)</h3>" <<std::endl;
            std::cout << "<h3>" << a << " % " << b << " = " << -1 << "(div zero)</h3>" <<std::endl;
        }else{
            std::cout << "<h3>" << a << " / " << b << " = " <<a / b << "</h3>" <<std::endl;
            std::cout << "<h3>" << a << " % " << b << " = " << a % b << "</h3>" <<std::endl;
        }

        std::cout << "</body>" << std::endl;
        std::cout << "</html>" << std::endl;

	return 0;

}
