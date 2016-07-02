#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/time.h>
#include <unistd.h>


#include<string.h>
#include<pthread.h>
#include<stdlib.h>

#define MAXCLIENTS	10

int threadcomplete=0;

struct ClientStatus{
	int IsConnected;
	pthread_t ThreadId;
	int ClientSocketFileDiscriptor,ClientLength;
	struct sockaddr_in ClientAddress;
	char Buffer[256];
};

void error(char *msg)
{
	printf(msg);
//	exit(1);
}



int main(int argc, char *argv[])
{
	struct ClientStatus Clients[MAXCLIENTS+1];
	int sockfd,newsockfd,portno,clilen,n,ClientCount=0;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;

	struct timeval tv;
	fd_set readfds,exceptfds;
	
	for(ClientCount=0;ClientCount<MAXCLIENTS+1;ClientCount++)
	{
		Clients[ClientCount].IsConnected=0;
	}

	if(argc < 2)
	{
		fprintf(stderr,"error, no port provided");
		error("Error, no port provided");
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error("ERROR opening socket");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port=htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))<0)
	{
		error("Error on binding");
	}
	listen(sockfd,5);
	while(1)
	{
		int maxid=0;
		tv.tv_sec=0;
		tv.tv_usec=500;
		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		FD_SET(0, &readfds);
		FD_SET(sockfd, &readfds);
		FD_SET(sockfd, &exceptfds);
		maxid=sockfd;
		for(ClientCount=0;ClientCount<10;ClientCount++)
		{
			if(Clients[ClientCount].IsConnected==1)
			{
				FD_SET(Clients[ClientCount].ClientSocketFileDiscriptor, &readfds);
				FD_SET(Clients[ClientCount].ClientSocketFileDiscriptor, &exceptfds);
				if(Clients[ClientCount].ClientSocketFileDiscriptor>maxid)
				{
					maxid=Clients[ClientCount].ClientSocketFileDiscriptor;
				}
			}
		}
		select(maxid+1,&readfds,NULL,&exceptfds,&tv);
		if(FD_ISSET(0,&readfds))
		{
			char data;
			scanf("%c",&data);
			printf("Closing Server\n");
			if(data=='C')
			{
				for(ClientCount=0;ClientCount<MAXCLIENTS;ClientCount++)
				{
					if(Clients[ClientCount].IsConnected==1)
					{
						close(Clients[ClientCount].ClientSocketFileDiscriptor);
					}
				}
				close(sockfd);
				return 0;
			}
		}
		if(FD_ISSET(sockfd,&readfds))
		{
			
//			printf("accepting Connection\n");
			for(ClientCount=0;ClientCount<MAXCLIENTS+1;ClientCount++)
			{
				if(Clients[ClientCount].IsConnected==0)
				{
					break;
				}else
				{
//					printf("Client point %d is busy %d\n",ClientCount,Clients[ClientCount].IsConnected);
				}
			}

//			printf("Accepting Client on %d\n",ClientCount);
			Clients[ClientCount].ClientLength = sizeof(Clients[ClientCount].ClientAddress);
			Clients[ClientCount].ClientSocketFileDiscriptor = accept(sockfd, (struct sockaddr *) &(Clients[ClientCount].ClientAddress),&(Clients[ClientCount].ClientLength));
			if(newsockfd < 0)
			{
				error("ERROR on accept");
			}else
			{
				if(ClientCount!=MAXCLIENTS)
				{
					Clients[ClientCount].IsConnected=1;
				}else
				{			
					printf("cannot Accomodate any more clients\n");
					close(Clients[ClientCount].ClientSocketFileDiscriptor);
				}
			}
		}
		
		for(ClientCount=0;ClientCount<MAXCLIENTS;ClientCount++)
		{	
			if((Clients[ClientCount].IsConnected==1)&&(FD_ISSET(Clients[ClientCount].ClientSocketFileDiscriptor,&exceptfds)))
			{
//				printf("Client Closed Connection on Client number %d\n",ClientCount);
				close(Clients[ClientCount].ClientSocketFileDiscriptor);
				Clients[ClientCount].IsConnected==0;
			}
			if((Clients[ClientCount].IsConnected==1)&&(FD_ISSET(Clients[ClientCount].ClientSocketFileDiscriptor,&readfds)))
			{
				int errorsoc = 0;
				socklen_t len = sizeof (errorsoc);
				int retval = getsockopt (Clients[ClientCount].ClientSocketFileDiscriptor
							, SOL_SOCKET
							, SO_ERROR
							, &errorsoc
							, &len);
				if((retval!=0)||(errorsoc!=0))
				{
//					printf("Client Closed Connection on Client number %d\n",ClientCount);
					close(Clients[ClientCount].ClientSocketFileDiscriptor);
					Clients[ClientCount].IsConnected=0;
					continue;
				}
				bzero(buffer,256);
				n=read(Clients[ClientCount].ClientSocketFileDiscriptor,buffer,255);
				if ( n <0 )
				{
					error("ERROR reading from socket\n");
				}
//				printf("Here is the message received from cleint %d : %s\n",ClientCount,buffer);
				n = write(Clients[ClientCount].ClientSocketFileDiscriptor, "I got your message",18);
				if(n<0)
				{
					error("ERROR writing to socket\n");
				}
			}
		}
	}
	return 0;
}
