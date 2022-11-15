/* A P2P client
It provides the following functions:
- Register the content file to the index server (R)
- Contact the index server to search for a content file (D)
	- Contact the peer to download the file
	- Register the content file to the index server
- De-register a content file (T)
- List the local registered content files (L)
- List the on-line registered content files (O)
*/

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>
#include <arpa/inet.h>

#define QUIT "quit"
#define SERVER_PORT 10000 /* well-known port */
#define BUFLEN 100		  /* buffer length */
#define NAMESIZ 20
#define MAXCON 200

typedef struct
{
	char type;
	char data[BUFLEN];
} PDU;
PDU rpdu;

struct
{
	int val;
	char name[NAMESIZ];
} table[MAXCON]; // Keep Track of the registered content

char usr[NAMESIZ];

int serverSock, peer_port;
int fd, nfds;
fd_set rfds, afds;

void registration(int, char *, struct sockaddr_in, int);
int search_content(int, char *, PDU *);
int client_download(char *, PDU *);
void server_download();
void deregistration(int, char *);
void online_list(int);
void local_list();
void quit(int);
void handler();

int main(int argc, char **argv)
{
	int s_port = SERVER_PORT;
	int n;
	int alen = sizeof(struct sockaddr_in);
	struct hostent *hp;
	// UDP server object
	struct sockaddr_in server, sin;
	char c, *host, name[NAMESIZ];
	struct sigaction sa;
	// Data buf
	char buf[100];

	switch (argc)
	{
	case 2:
		host = argv[1];
		break;
	case 3:
		host = argv[1];
		s_port = atoi(argv[2]);
		break;
	default:
		printf("Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* UDP Connection with the index server		*/
	memset(&server, 0, alen);
	server.sin_family = AF_INET;
	server.sin_port = htons(s_port);
	if (hp = gethostbyname(host))
		memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	else if ((server.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
	{
		printf("Can't get host entry \n");
		exit(1);
	}
	serverSock = socket(PF_INET, SOCK_DGRAM, 0); // Allocate a socket for the index server
	if (serverSock < 0)
	{
		printf("Can't create socket \n");
		exit(1);
	}
	if (connect(serverSock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("Can't connect \n");
		exit(1);
	}

	/* 	Enter User Name		*/
	printf("Choose a user name\n");
	scanf("%s", usr);

	/* Initialization of SELECT`structure and table structure	*/
	FD_ZERO(&afds);
	FD_SET(serverSock, &afds); /* Listening on the index server socket  */
	FD_SET(0, &afds);		   /* Listening on the read descriptor   */
	nfds = 1;
	for (n = 0; n < MAXCON; n++)
		table[n].val = -1;

	/*	Setup signal handler		*/
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	/* Main Loop	*/
	while (1)
	{
		printf("Command:\n");

		memcpy(&rfds, &afds, sizeof(rfds));
		if (select(nfds, &rfds, NULL, NULL, NULL) == -1)
		{
			printf("select error: %s\n", strerror(errno));
			exit(1);
		}

		if (FD_ISSET(0, &rfds))
		{ /* Command from the user  */
			c = getchar();

			/*	Command options	*/
			if (c == '?')
			{
				printf("R-Content Registration; T-Content Deregistration; L-List Local Content\n");
				printf("D-Download Content; O-List all the On-line Content; Q-Quit\n\n");
				continue;
			}

			/*	Content Regisration	*/
			if (c == 'R')
			{
				registration(serverSock, usr, sin, alen);
			}

			/*	List Content		*/
			if (c == 'L')
			{
				/* Call local_list()	*/
			}

			/*	List on-line Content	*/
			if (c == 'O')
			{
				/* Call online_list()	*/
			}

			/*	Download Content	*/
			if (c == 'D')
			{
				/* Call search_content()	*/
				/* Call client_download()	*/
				/* Call registration()		*/
			}

			/*	Content Deregistration	*/
			if (c == 'T')
			{
				/* Call deregistration()	*/
			}

			/*	Quit	*/
			if (c == 'Q')
			{
				/* Call quit()	*/
			}
		}

		/* Content transfer: Server to client		*/
		else
			server_download(serverSock);
	}
}

void quit(int serverSock)
{
	/* De-register all the registrations in the index server	*/
}

void local_list()
{
	/* List local content	*/
}

void online_list(int serverSock)
{
	/* Contact index server to acquire the list of content */
}

void server_download()
{
	/* Respond to the download request from a peer	*/
}

int search_content(int serverSock, char *name, PDU *rpdu)
{
	/* Contact index server to search for the content
	   If the content is available, the index server will return
	   the IP address and port number of the content server.	*/
}

int client_download(char *name, PDU *pdu)
{
	/* Make TCP connection with the content server to initiate the
	   Download.	*/
}

void deregistration(int serverSock, char *name)
{
	/* Contact the index server to deregister a content registration;	   Update nfds. */
}

void registration(int serverSock, char *name, struct sockaddr_in server, int alen)
{
	/* Create a TCP socket for content download
			� one socket per content;
	   Register the content to the index server;
	   Update nfds;	*/
	struct sockaddr_in tcpAddr;
	int tcpSock, myIP;
	unsigned int myPort;
	char buf[100];
	PDU rPacket;
	int n;

	// Initializing tcp socket
	bzero(&tcpAddr, sizeof(tcpAddr));
	socklen_t len = sizeof(tcpAddr);
	tcpSock = socket(AF_INET, SOCK_STREAM, 0);
	tcpAddr.sin_family = AF_INET;
	tcpAddr.sin_port = htons(0);
	tcpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(tcpSock, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr));

	// Assigning randomized tcp port
	getsockname(tcpSock, (struct sockaddr *)&tcpAddr, &len);
	inet_ntop(AF_INET, &tcpAddr.sin_addr, myIP, sizeof(myIP));

	myPort = ntohs(tcpAddr.sin_port);
	char *ip2 = inet_ntoa(((struct sockaddr_in *)&tcpAddr)->sin_addr);
	printf("Local ip address: %s\n", ip2);
	printf("Local port: %u\n", myPort);
	fflush(stdout);

	// Transferring peer name, content name, tcp ip address, tcp port info into buf variable
	strcpy(buf, name);
	memset(buf + strlen(buf), ' ', 100 - strlen(buf) - 1);

	printf("buf: %s\n", buf);
	fflush(stdout);
	printf("Enter content name: ");
	scanf("%s", &buf[9]);

	memset(buf + strlen(buf), ' ', 100 - strlen(buf) - 1);
	strcpy(buf + 19, ip2);
	memset(buf + strlen(buf), ' ', 100 - strlen(buf) - 1);
	sprintf(buf + 19 + strlen(ip2) + 1, "%d", myPort);

	printf("buffer: %s\n", buf);
	fflush(stdout);

	// Initializing register packet with R type and buf data
	rPacket.type = 'R';
	strcpy(rPacket.data, buf);

	// Send register packet to server
	write(serverSock, &rPacket, sizeof(PDU));
	alen = sizeof(server);

	// Receive acknowledgement or error from server
	int k = recvfrom(serverSock, &rpdu, sizeof(PDU), 0,
					 (struct sockaddr *)&server, &alen);
	if (k < 0)
	{
		fprintf(stderr, "Read failed: %d, %s\n", k, strerror(errno));
	}

	printf("received: %s, %c\n", rpdu.data, rpdu.type);
	fflush(stdout);

	if (rpdu.type == 'A')
	{
		printf("Content registered successfully!\n");
	}
	else if (rpdu.type == 'E')
	{
		printf("Error while registering content. Content might already be registered.\n");
	}
}

void handler()
{
	quit(serverSock);
}
