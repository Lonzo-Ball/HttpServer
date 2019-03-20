
bin=HttpdServer
cc=g++

.PHONY:all
all:$(bin) cgi
$(bin):HttpdServer.cc
	$(cc) -o $@ $^ -lpthread -std=c++11

.PHONY:cgi
cgi:
	g++ -o Cal Cal.cc
