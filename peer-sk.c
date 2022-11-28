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
#include <strings.h>
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
	int tcpSock;
	char name[NAMESIZ];
} registeredContent[MAXCON]; // Keep Track of the registered content

char usr[NAMESIZ];

int serverSock, peer_port;
int fd, nfds;
fd_set rfds; // Indicates which socket(s) are required for service
fd_set afds;
int new_sd;
fd_set current_sockets, ready_sockets;
int maxRegContent = 0;

void registration(int, char *, struct sockaddr_in, int);
char *search_content(int, char *, struct sockaddr_in, int);
void client_download(char *);
void server_download();
void deregistration(int, char *);
void online_list(int);
void local_list();
void quit(int);
void handler();
void printRegisteredContent();
void *receive_thread(void *);
void receiving(int);
void reaper(int);
int echod(int);

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
	maxRegContent = 0;
	int new_socket;

	struct sockaddr_in tcpServer;
	int tcpalen;

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

	printf("ServerSocket: %d\n", serverSock);
	/* 	Enter User Name		*/
	printf("Choose a user name\n");
	scanf("%s", usr);

	/* Initialization of SELECT`structure and table structure	*/
	// FD_ZERO(&afds);
	// FD_SET(serverSock, &afds); /* Listening on the index server socket  */
	// FD_SET(0, &afds);		   /* Listening on the read descriptor   */

	// FD_ZERO(&current_sockets);
	nfds = 1;
	for (n = 0; n < MAXCON; n++)
		registeredContent[n].tcpSock = -1;

	/*	Setup signal handler		*/
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags;
	sigaction(SIGINT, &sa, NULL);

	/* Main Loop	*/
	while (1)
	{
		printf("Command:\n");
		// memcpy(&rfds, &afds, sizeof(rfds));
		// if (select(nfds, &rfds, NULL, NULL, NULL) == -1)
		// {
		// 	printf("select error: %s\n", strerror(errno));
		// 	exit(1);
		// }

		// if (FD_ISSET(0, &rfds))
		// { /* Command from the user  */
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
			char *contentAddrInfo = NULL;
			// strcpy(contentAddrInfo, search_content(serverSock, usr, sin, alen));
			contentAddrInfo = search_content(serverSock, usr, sin, alen);
			printf("contentAddrInfo: %s\n", contentAddrInfo);
			if (contentAddrInfo != NULL)
			{
				// Content address info retrieved
				client_download(contentAddrInfo);
			}
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
		// }

		/* Content transfer: Server to client		*/
		// else
		// {
		// 	server_download(serverSock);
		// }
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
	printf("\nwe in server_download()\n");
	fflush(stdout);
	// Iterate through locally registered content
	// 	find the matching content name, use FD logic
	// 	to select the TCP socket described there
}

/**
 * @brief - attempts to connect to TCP content server based on ip address & port of it
 *
 * @param clientAddrInfo - in the format of "<ipAddress> <portNumber>"
 * @return int
 */
void client_download(char *clientAddrInfo)
{
	struct hostent *he;
	int alen;
	/* Make TCP connection with the content server to initiate the
	   Download.	*/
	printf("Sending client download packet:\n");
	// Parse clientAddrInfo
	char *ipAddr, *port;
	ipAddr = strtok(clientAddrInfo, " ");
	port = strtok(NULL, " ");

	// strcpy(ipAddr, "0.0.0.0");
	printf("parsed clientAddrInfo: %s, %s\n", ipAddr, port);
	// Send TCP packet to client server
	int serverTcpSock;
	struct sockaddr_in tcpServer;
	struct hostent *hp;

	if ((serverTcpSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Cant create a socket\n");
		return;
	}

	bzero((char *)&tcpServer, sizeof(struct sockaddr_in));
	tcpServer.sin_family = AF_INET;
	tcpServer.sin_port = ntohs(atoi(port));
	if (hp = gethostbyname(ipAddr))
	{
		bcopy(hp->h_addr, (char *)&tcpServer.sin_addr, hp->h_length);
	}
	else if (inet_aton(ipAddr, (struct in_addr *)&tcpServer.sin_addr))
	{
		fprintf(stderr, "Cant get TCP content server address\n");
		return;
	}
	alen = sizeof(tcpServer);
	if (connect(serverTcpSock, (struct sockaddr *)&tcpServer, alen) == -1)
	{
		fprintf(stderr, "Cant connect to TCP content server, %s\n", strerror(errno));
		return;
	}

	printf("Transmit to tcp sock:%d \n", serverTcpSock);
	// TODO: DOES THIS WORK? How would you get listen for this write?
	// 	Solution -> gotta use FDSET to somehow set the socket that you're
	// 		listening to, see open web browser for it
	char buf3[100];
	strcpy(buf3, "helloFromPeer");

	write(serverTcpSock, buf3, 100);
	return;
}
/**
 * @brief
 *
 * @param serverSock
 * @param name
 * @param server
 * @param alen
 * @return char* - a string that is NULL if no content is found
 * 				- if content is found, returns a string of the format "<ipAddress> <portNumber>"
 */
char *search_content(int serverSock, char *name, struct sockaddr_in server, int alen)
{
	/* Contact index server to search for the content
	   If the content is available, the index server will return
	   the IP address and port number of the content server.	*/
	PDU spdu;

	spdu.type = 'S';

	strcpy(spdu.data, name);
	strcat(spdu.data, " ");
	printf("Enter content name you want to search:");
	scanf("%s", spdu.data + strlen(spdu.data));

	printf("searching spdu data: %s\n", spdu.data);
	fflush(stdout);
	// Send S PDU packet
	write(serverSock, &spdu, sizeof(PDU));
	alen = sizeof(server);

	int k = recvfrom(serverSock, &rpdu, sizeof(PDU), 0,
					 (struct sockaddr *)&server, &alen);
	if (k < 0)
	{
		fprintf(stderr, "Read failed: %d, %s\n", k, strerror(errno));
	}

	if (rpdu.type == 'E')
	{
		printf("%s\n", rpdu.data);
		return NULL;
	}
	else if (rpdu.type == 'S')
	{
		printf("Content successfully found: %s\n", rpdu.data);
		// Convert char[] to char * due to return value type (also return value type
		// 		of char[] did not seem to work)
		char *buf = rpdu.data;
		return buf;
	}
	return NULL;
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
	int tcpSock;
	char myIP[16];
	unsigned int myPort;
	char buf[100];
	PDU rPacket;
	int n;
	char contentName[10];

	// Initializing tcp socket
	bzero(&tcpAddr, sizeof(tcpAddr));
	socklen_t len = sizeof(tcpAddr);
	tcpSock = socket(AF_INET, SOCK_STREAM, 0);
	tcpAddr.sin_family = AF_INET;
	tcpAddr.sin_port = htons(0);
	tcpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	// tcpAddr.sin_addr.s_addr = inet_addr("10.0.0.251");
	if (bind(tcpSock, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr)) == -1)
	{
		fprintf(stderr, "Can't bind name to socket\n");
		return;
	}

	// Assigning randomized tcp port
	getsockname(tcpSock, (struct sockaddr *)&tcpAddr, &len);
	inet_ntop(AF_INET, &tcpAddr.sin_addr, myIP, sizeof(myIP));

	myPort = htons(tcpAddr.sin_port);
	// printf("Actual port (?????): %d\n", htons(tcpAddr.sin_port));
	char *ip2 = inet_ntoa(((struct sockaddr_in *)&tcpAddr)->sin_addr);
	printf("Local ip address: %s, myIP: %s\n", ip2, myIP);
	printf("Local port: %u\n", myPort);
	fflush(stdout);

	// Transferring peer name, content name, tcp ip address, tcp port info into buf variable
	strcpy(buf, name);
	memset(buf + strlen(buf), ' ', 100 - strlen(buf) - 1);

	printf("buf: %s\n", buf);
	fflush(stdout);
	printf("Enter content name: ");
	scanf("%s", contentName);
	strcpy(&buf[9], contentName);
	// scanf("%s", &buf[9]);

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
		// Add the port/address info to local registeredContent list
		registeredContent[maxRegContent].tcpSock = tcpSock;
		strcpy(registeredContent[maxRegContent].name, contentName);
		maxRegContent++;

		// nfds++;
		// FD_SET(tcpSock, &afds);
		if (listen(tcpSock, 5) < 0)
		{
			perror("listen");
			fprintf(stderr, "Error listening to ports\n");
		}
		//(void)signal(SIGCHLD, reaper);

		int n;
		char buf2[100];
		// memset(buf2, '\0', sizeof(char) * 100);
		struct sockaddr_in client;
		int client_len = sizeof(struct sockaddr_in);
		switch (fork())
		{
		case 0:
			while (1)
			{
				printf("we in the forked child\n");
				// if (
				new_sd = accept(tcpSock, (struct sockaddr *)&client, &client_len);
				// ) < 0)
				// {
				// fprintf(stderr, "Cannot accept client\n");
				// exit(1);
				// }
				printf("accepted new client, waiting for message, %d\n", new_sd);
				//				n = recv(new_sd, buf2, 100);

				// printf("packet received from tcp: %s\n", buf2);
				// while (1)
				// {
				n = recv(new_sd, &buf2, 100, 0);
				// if (n < 0)
				// continue;
				printf("packet received from tcp client: %s\n", buf2);
				fflush(stdout);
				// }
			}
			close(new_sd);
			// child listens to the TCP connection
			// while (1)
			// {
			// }
			break;
		case -1:
			fprintf(stderr, "Error was found while creating child process");
			break;
		}
		// pthread_t tid;
		// //  Creating thread to keep receiving messages in real time
		// printf("creating thread with tcpSock: %d\n", tcpSock);
		// pthread_create(&tid, NULL, receive_thread, (void *)tcpSock);

		printRegisteredContent();
	}
	else if (rpdu.type == 'E')
	{
		printf("Error while registering content. Content might already be registered.\n");
	}
}
/*	reaper		*/
void reaper(int sig)
{
	int status;
	while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
		;
}

void *receive_thread(void *arg)
{
	int tcpSock = (int)arg;
	while (1)
	{
		sleep(2);
		receiving(tcpSock);
	}
}

void receiving(int tcpSock)
{
	int new_sd;
	struct sockaddr_in client;
	int client_len = sizeof(struct sockaddr_in);
	printf("Receive_thread: tcpSock: %d\n", tcpSock);

	// accept tcp client connection
	while (1)
	{
		if (new_sd = accept(tcpSock, (struct sockaddr *)&client, &client_len) < 0)
		{
			fprintf(stderr, "Cannot accept client\n");
			exit(1);
		}
		switch (fork())
		{
		case 0:
			(void)close(tcpSock);
			exit(echod(new_sd));
			break;
		default:
			(void)close(new_sd);
			break;
		case -1:
			fprintf(stderr, "fork: error\n");
		}
	}
}

int echod(int sd)
{
	int n;
	char buf[100];
	while (n = read(sd, buf, 100))
	{

		printf("packet read: %s\n", buf);
		fflush(stdout);
	}

	close(sd);
	return (0);
}

void handler()
{
	quit(serverSock);
}

void printRegisteredContent()
{
	int i = 0;
	printf("\nPrinting registered content\n");
	for (i = 0; i < MAXCON; i++)
	{
		if (registeredContent[i].tcpSock == -1)
			break;
		printf("\tContent name: %s, sock: %d\n", registeredContent[i].name, registeredContent[i].tcpSock);
	}
	return;
}