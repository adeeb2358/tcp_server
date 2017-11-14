#include "bs_tcp_bigHeader.h"

//read incoming data from the input file descriptor
std::string bs_tcp_readInComingData(int incoming_file_descriptor,int epoll_file_descriptor){
	struct epoll_event event;
	std::string output_string = "";
	int data_flag = 0;
	char output_buffer[512];
	int count;
	while(1){
		std::string temp_string = "";
		memset(output_buffer, 0, 512);
		count = read (incoming_file_descriptor, output_buffer, sizeof output_buffer);
		std::cout <<"count_value=>"<<count<<" ";
		if(count>0){
			
			data_flag     = 1;
			temp_string   = output_buffer;
			output_string += temp_string;
			output_string.erase(output_string.find_last_not_of(" \n\r\t")+1);

		}else if(count < 0){
			break;
		}else if(count == 0){
			//int execution_result = bs_tcp_makeSocketBlocking(incoming_file_descriptor);
			int execution_result = close(incoming_file_descriptor);
			std::cout <<"Close=>" << execution_result <<" ";
			execution_result = epoll_ctl(epoll_file_descriptor, EPOLL_CTL_DEL, incoming_file_descriptor, &event);
			std::cout << "epoll=>" <<execution_result << " ";
			std::cout <<"Disconnected =>"<< incoming_file_descriptor <<"\n";
			break;
		}

	}
	if(data_flag == 1){
		return output_string;
	}
	return "-1";
}

/*processing incoming data*/
void bs_tcp_processIncomingData(int incoming_file_descriptor,int epoll_file_descriptor){
	struct sockaddr client_ip_address;
	char incoming_ip_address[512];
	char incoming_port_no[512];

	//code for getting client ip address and port number
	socklen_t client_ip_address_length = sizeof(client_ip_address);
	int result                         = getpeername(incoming_file_descriptor,&client_ip_address,&client_ip_address_length);
	if(result == -1){
		return;
	}

	result = getnameinfo (&client_ip_address, client_ip_address_length,
                          incoming_ip_address, sizeof incoming_ip_address,
                          incoming_port_no, sizeof incoming_port_no,
                          NI_NUMERICHOST | NI_NUMERICSERV);
	if(result == -1){
		return;
	}

	std::string incoming_text = bs_tcp_readInComingData(incoming_file_descriptor,epoll_file_descriptor);
	if(incoming_text == "-1"){
		return;
	}

	std::string incoming_ip_address_string = incoming_ip_address;
	std::string client_output_string       = "incoming_text_from " + incoming_ip_address_string +std::string(":")+incoming_port_no+ std::string("=>") + incoming_text+ std::string("\n");
	std::cout << client_output_string << "\n";
	result = write(incoming_file_descriptor,client_output_string.c_str(),client_output_string.size());
	return;
}

/*processing incoming connection*/
void bs_tcp_processIncomingConnection(int socket_file_descriptor,int epoll_file_descriptor){
	struct epoll_event event;

	std::string error_message = "";
	std::string date_time     = "";

	int execution_result;
	int incoming_file_descriptor;
	struct sockaddr incoming_address;
	socklen_t incoming_socket_length;
	
	char incoming_ip_address[512];
	char incoming_port_no[512];
		

	incoming_socket_length   = sizeof(incoming_address);
	incoming_file_descriptor = accept(socket_file_descriptor,&incoming_address,&incoming_socket_length);	
	
	

	if(incoming_file_descriptor == -1){
		//check whether we have processed all the incoming request
        int process_result = (errno == EAGAIN)||(errno == EWOULDBLOCK);
        if(process_result){
        	return ;				
        }else{
			error_message = "Connection Accept Error";
			date_time     = bs_mn_genTime();
        	bs_tcp_writeLogs(1,error_message,date_time);
        }
		return ;
	}

	execution_result = bs_tcp_makeSocketNonBlocking(incoming_file_descriptor);
 	if(execution_result == -1){
 		//if we cant make the socket connection non blocking just close the connection
 		close(incoming_file_descriptor);
 		incoming_file_descriptor = -1;
 		return ;
 	}

 	
	
	event.data.fd = incoming_file_descriptor;
	event.events  = EPOLLIN | EPOLLET;
	execution_result = epoll_ctl (epoll_file_descriptor, EPOLL_CTL_ADD, incoming_file_descriptor, &event);
	if(execution_result == -1){
		error_message = "Incoming Socket Addition to epoll watch list failed";
		date_time     = bs_mn_genTime();
		bs_tcp_writeLogs(1,error_message,date_time);
		close(incoming_file_descriptor);
		return ;
	}

	//end of addition

	//start of code
	execution_result = getnameinfo (&incoming_address, incoming_socket_length,
                          incoming_ip_address, sizeof incoming_ip_address,
                          incoming_port_no, sizeof incoming_port_no,
                          NI_NUMERICHOST | NI_NUMERICSERV); 

	//end of code
    
    std::cout <<incoming_ip_address<<":"<<incoming_port_no<<" Connected=>"<<incoming_file_descriptor<<"\n";
	return ;
}
/*end of code*/

/*code for getting the file size*/
size_t bs_mn_checkFileSize(std::string file_name){
	size_t file_size = -1;
	struct stat st; 
	if (stat(file_name.c_str(), &st) == 0){
        return st.st_size;
	}
	return file_size;
}
/*end of code*/

/*function for generating date and time*/
std::string bs_mn_genTime(){
	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (buffer,80,"Date: %a, %d %h %Y %H:%M:%S +0000",timeinfo);
	return buffer;
}
/*end of code*/

/*function to write logs for fail cases
if log_type = 0 ,its a success message and will be logged to the success.log file
if log_type = 1, its an error message and will be logged to the error.log file
*/
void bs_tcp_writeLogs(int log_type,std::string error_message,std::string date_time){
	std::string file_name =  "success.log";
	if(log_type == 1){
		file_name = "error.log";
	}
	std::ofstream out_file;
	size_t out_file_size=  bs_mn_checkFileSize(file_name);
	if(out_file_size == -1){
		out_file.open(file_name);
	}else{
		out_file.open(file_name,std::ios::app);
	}
	out_file << "message:-"<< error_message;
	out_file << "\t";
	out_file << "date_time:-"<< date_time<<"\n";
	out_file.close();
	return;
}
/*end of code*/

/*function for making socket  blocking*/
int bs_tcp_makeSocketBlocking(int socket_file_descriptor){
	
	std::string error_message = "";
	std::string date_time     = "";

	int flags;
	int fcntl_result;

	flags = fcntl (socket_file_descriptor, F_GETFL, 0);	
	if(flags == -1){
		error_message = "Socket  Blocking Failed";
		date_time     = bs_mn_genTime();
		bs_tcp_writeLogs(1,error_message,date_time);
		return -1;
	}

	flags       ^= O_NONBLOCK;
	fcntl_result = fcntl (socket_file_descriptor, F_SETFL, flags);
	if(fcntl_result == -1){
		error_message = "Socket  Blocking Failed fcntl";
		date_time     = bs_mn_genTime();
		bs_tcp_writeLogs(1,error_message,date_time);
		return -1;
	}

	return 0;
}


/*function for making socket non blocking*/
int bs_tcp_makeSocketNonBlocking(int socket_file_descriptor){
	
	std::string error_message = "";
	std::string date_time     = "";

	int flags;
	int fcntl_result;

	flags = fcntl (socket_file_descriptor, F_GETFL, 0);	
	if(flags == -1){
		error_message = "Socket Non Blocking Failed";
		date_time     = bs_mn_genTime();
		bs_tcp_writeLogs(1,error_message,date_time);
		return -1;
	}

	flags       |= O_NONBLOCK;
	fcntl_result = fcntl (socket_file_descriptor, F_SETFL, flags);
	if(fcntl_result == -1){
		error_message = "Socket Non Blocking Failed fcntl";
		date_time     = bs_mn_genTime();
		bs_tcp_writeLogs(1,error_message,date_time);
		return -1;
	}

	return 0;
}

/*main function to start the tcp server*/
void bs_tcp_startTCPServer(std::string bind_ip,std::string bind_port,int connection_queue_backlog_size,int max_events){
	
	struct sockaddr_in server_address;
	int socket_file_descriptor;
	
	int bind_result;
	int result;
	
	int epoll_file_descriptor;
	struct epoll_event event;
	struct epoll_event *events;
	

	std::string error_message = "";
	std::string date_time     = "";

	// /*creating a socket file descriptor*/
	// socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	// if(socket_file_descriptor < 0){
	// 	std::cout << "Creating Socket Failed\n";
	// 	exit(0);
	// }
	
	// /*initializing server address structure*/
	// memset(&server_address, 0, sizeof(server_address));
	// server_address.sin_family      = AF_INET;
	// server_address.sin_addr.s_addr = inet_addr(bind_ip.c_str());
	// server_address.sin_port        = htons(atoi(bind_port.c_str()));

	// int yes = 1;
	// if ( setsockopt(socket_file_descriptor, SOCK_STREAM, SO_REUSEADDR, &yes, sizeof(int)) == -1 ){
 //    	std::cout <<"Socket Option Set Error\n";
	// 	exit(0);
	// }
	
	// /*binding the ip address with the socket*/
	// bind_result = bind(socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address));
	// if(bind_result < 0){
	// 	std::cout << "Bind Failed\n";
	// 	exit(0);
	// }
	
	socket_file_descriptor = create_and_bind(bind_ip,bind_port);
	std::cout << socket_file_descriptor <<"\n";
	/*code for making socket non blocking*/
	result = bs_tcp_makeSocketNonBlocking(socket_file_descriptor);
	if(result == -1){
		std::cout << "non blocking failed\n";
		//socket nonblocking making failed
		exit(0);
	}

	/*listening for incoming connections*/
	result = listen(socket_file_descriptor,connection_queue_backlog_size);
	if(result < 0){
		std::cout <<"listen error\n";
		exit(0);
	}	

	epoll_file_descriptor = epoll_create1(0);

	if(epoll_file_descriptor  == -1){
	//epoll creation failed
		error_message  =  "Epoll Creation Failed";
		date_time      =   bs_mn_genTime();
		bs_tcp_writeLogs(1,error_message,date_time);
		std::cout << error_message <<"\n";
		exit(0);
	}

	//registering listening socket with epoll and  making epoll edge triggered mode
	event.data.fd = socket_file_descriptor;
	event.events  = EPOLLIN | EPOLLET;

	//adding the server socket file descriptor to the epoll events
	result = epoll_ctl(epoll_file_descriptor,EPOLL_CTL_ADD,socket_file_descriptor,&event);


	if(result == -1){
		//epoll control function failed
		error_message  =  "Epoll Control Function Failed(Adding Socket File Descriptor)";
		date_time      =   bs_mn_genTime();
		bs_tcp_writeLogs(1,error_message,date_time);
		std::cout << error_message <<"\n";
		exit(0);
	}

	events = (epoll_event *)calloc(max_events,sizeof(event));
	/*epoll wait event loop*/
	while(1){
		
		int no_of_file_descriptors = epoll_wait(epoll_file_descriptor,events,max_events,-1);
		//loop through each event and process it
		for(int i=0;i<no_of_file_descriptors;i++){
			int error_condition = (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN));
            
            if(error_condition){
            	error_message = "Epoll Error Error Condition";
				date_time     = bs_mn_genTime();
        		bs_tcp_writeLogs(1,error_message,date_time);
        		epoll_ctl(epoll_file_descriptor, EPOLL_CTL_DEL, events[i].data.fd, &event);
				close (events[i].data.fd);
	      		continue;
            }else if(socket_file_descriptor == events[i].data.fd){
            	//bs_tcp_processIncomingConnection(socket_file_descriptor,epoll_file_descriptor);
            	std::thread incoming_connection_thread (bs_tcp_processIncomingConnection,socket_file_descriptor,epoll_file_descriptor);	
            	incoming_connection_thread.detach();
            	continue;
            }else{
            	int incoming_file_descriptor = events[i].data.fd;
            	//bs_tcp_processIncomingData(incoming_file_descriptor,epoll_file_descriptor);
            	std::thread client_message_process_thread(bs_tcp_processIncomingData,incoming_file_descriptor,epoll_file_descriptor);
            	client_message_process_thread.detach();
            	continue;
            }
		}//end of for loop		
		
	}
	
	epoll_ctl(epoll_file_descriptor, EPOLL_CTL_DEL, socket_file_descriptor, &event);
	close(socket_file_descriptor);
	free(events);
	return;
}



int create_and_bind (std::string bind_ip,std::string bind_port){
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */

  s = getaddrinfo (bind_ip.c_str(), bind_port.c_str(), &hints, &result);
  if (s != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
      return -1;
    }

  for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
        {
          /* We managed to bind successfully! */
          break;
        }

      close (sfd);
    }

  if (rp == NULL)
    {
      fprintf (stderr, "Could not bind\n");
      return -1;
    }

  freeaddrinfo (result);

  return sfd;
}