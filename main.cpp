#include "bs_tcp_bigHeader.h"


/*
httperf --client=0/1 --server=localhost --port=80 --uri=/ --send-buffer=4096 --recv-buffer=16384 --num-conns=1 --num-calls=1

httperf --hog --server=192.168.2.2  --port=1024  --wsess=100000,5,2 --rate 1000 
*/


int main(){
	std::cout <<"Starting Tcp Server\n";
	std::string bind_ip               = "172.20.10.2";
	std::string bind_port             = "1024";
	int connection_queue_backlog_size = 100;
	int max_events                    = 5;
	bs_tcp_startTCPServer(bind_ip,bind_port,connection_queue_backlog_size,max_events);
	return 0;
}