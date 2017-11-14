#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <string.h>
#include <netinet/in.h>
#include <unistd.h>


#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr

#include <thread>


#include <netdb.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <fstream>
#include <thread>


#define CLADDR_LEN 100
#define BUF_SIZE   2000


void bs_tcp_startTCPServer(std::string bind_ip,std::string bind_port,int connection_queue_backlog_size,int max_events);
void bs_tcp_processClientMessage(int new_client_socket_file_descriptor, struct sockaddr_in client_address,int client_address_length);
int bs_tcp_makeSocketNonBlocking(int socket_file_descriptor);
void bs_tcp_writeLogs(int log_type,std::string error_message,std::string date_time);
std::string bs_mn_genTime();
size_t bs_mn_checkFileSize(std::string file_name);
void bs_tcp_processIncomingConnection(int socket_file_descriptor,int epoll_file_descriptor);
void bs_tcp_processIncomingData(int incoming_file_descriptor,int epoll_file_descriptor);
std::string bs_tcp_readInComingData(int incoming_file_descriptor,int epoll_file_descriptor);
int bs_tcp_makeSocketBlocking(int socket_file_descriptor);
int create_and_bind (std::string bind_ip,std::string bind_port);