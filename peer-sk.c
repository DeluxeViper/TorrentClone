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

void registration(int, char *, struct sockaddr_in, char *);
char *search_content(int, char *, struct sockaddr_in, int, char *);
void client_download(char *, char *, char *);
void server_download();
void deregistration(int, char *, struct sockaddr_in);
void online_list(int);
void local_list();
void quit(int);
void handler();
void printRegisteredContent();
void removeElementFromRegistration(int);

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

	char contentName[100];
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
			registration(serverSock, usr, sin, NULL);
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
			// Enter content name

			printf("Enter content name you want to search:");
			scanf("%s", contentName);

			char *contentAddrInfo = NULL;
			contentAddrInfo = search_content(serverSock, usr, sin, alen, contentName);
			printf("contentAddrInfo: %s\n", contentAddrInfo);
			if (contentAddrInfo != NULL)
			{
				// Content address info retrieved
				client_download(contentAddrInfo, usr, contentName);
				// register new content with index server
				registration(serverSock, usr, sin, contentName);
			}
			/* Call registration()		*/
		}

		/*	Content Deregistration	*/
		if (c == 'T')
		{
			/* Call deregistration()	*/
			printf("Enter content name you want to deregister:");
			scanf("%s", contentName);
			deregistration(serverSock, contentName, sin);
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
void client_download(char *clientAddrInfo, char *usr, char *contentName)
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
	// Send download packet to content server to retrieve
	PDU dPacket, cPacket;
	int isInitPacketRead = 0;
	FILE *fp;
	char *readBf = malloc(sizeof(char) * 100);
	bzero(dPacket.data, BUFLEN);
	strcat(dPacket.data, usr);
	strcat(dPacket.data, " ");
	strcat(dPacket.data, contentName);
	dPacket.type = 'D';

	write(serverTcpSock, &dPacket, sizeof(PDU));

	// Receive file
	printf("entering while loop\n");
	bzero(cPacket.data, sizeof(cPacket.data));
	bzero(readBf, sizeof(cPacket.data));
	int m;
	// Write to file based on receiving content packets
	while ((m = read(serverTcpSock, &cPacket, sizeof(PDU))) > 0)
	{
		strcpy(readBf, &cPacket.data);
		printf("type: %c, readBf: %s\n", cPacket.type, readBf);
		if (cPacket.type == 'E')
		{
			fprintf(stderr, "Error, file does not exist.\n");
			break;
		}
		else if (isInitPacketRead == 0)
		{
			// File does not exist and not an error
			printf("Opened file\n");
			fp = fopen(contentName, "w+");
		}
		// Write to the file
		readBf[strcspn(readBf, "\n")] = 0;
		readBf[strlen(readBf)] = '\0';
		printf("fp: %s", fp);
		fwrite(readBf, 100, 100, fp);
		isInitPacketRead = 1;
		printf("wrote to file\n");
		if (cPacket.type == 'F')
			break;
	}
	fclose(fp);
	return;
}
/**
 * @brief
 *
 * @param serverSock
 * @param name
 * @param server
 * @param alen
 * @param contentName
 * @return char* - a string that is NULL if no content is found
 * 				- if content is found, returns a string of the format "<ipAddress> <portNumber>"
 */
char *search_content(int serverSock, char *name, struct sockaddr_in server, int alen, char *contentName)
{
	/* Contact index server to search for the content
	   If the content is available, the index server will return
	   the IP address and port number of the content server.	*/
	PDU spdu;

	spdu.type = 'S';

	strcpy(spdu.data, name);
	strcat(spdu.data, " ");
	strcat(spdu.data, contentName);
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

void deregistration(int serverSock, char *name, struct sockaddr_in server)
{
	/* Contact the index server to deregister a content registration;	   Update nfds. */
	PDU tPacket, rpdu;
	// Tpacket -> T | usr | contentName
	tPacket.type = 'T';
	bzero(tPacket.data, BUFLEN);
	strcat(tPacket.data, usr);
	strcat(tPacket.data, " ");
	strcat(tPacket.data, name);

	printf("Attempting to deregister user: %s with content: %s\n", usr, name);
	write(serverSock, &tPacket, sizeof(PDU));
	int alen = sizeof(server);

	// Receive acknowledgement or error from server
	int k = recvfrom(serverSock, &rpdu, sizeof(PDU), 0,
					 (struct sockaddr *)&server, &alen);

	if (rpdu.type == 'A')
	{
		printf("Deregistered content: %s, maxRegContent: %d\n", name, maxRegContent);
		// Remove registered content from the list
		for (int i = 0; i < maxRegContent; i++)
		{
			printf("comparing reg content: %s, %s\n", registeredContent[i].name, name);
			if (strcmp(registeredContent[i].name, name) == 0)
			{
				printf("removing local registered content\n");
				removeElementFromRegistration(i);
				printRegisteredContent();
			}
		}
	}
	else if (rpdu.type == 'E')
	{
		printf("%s\n", rpdu.data);
	}
	return;
}

void removeElementFromRegistration(int pos)
{
	// shift all the element from index+1 by one position to the left
	for (int i = pos; i < maxRegContent - 1; i++)
		registeredContent[i] = registeredContent[i + 1];
	strcpy(registeredContent[maxRegContent].name, "");
	registeredContent[maxRegContent].tcpSock = 0;
	maxRegContent--;
}

void registration(int serverSock, char *name, struct sockaddr_in server, char *providedContentName)
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
	if (bind(tcpSock, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr)) == -1)
	{
		fprintf(stderr, "Can't bind name to socket\n");
		return;
	}

	// Assigning randomized tcp port
	getsockname(tcpSock, (struct sockaddr *)&tcpAddr, &len);
	inet_ntop(AF_INET, &tcpAddr.sin_addr, myIP, sizeof(myIP));

	myPort = htons(tcpAddr.sin_port);
	char *ip2 = inet_ntoa(((struct sockaddr_in *)&tcpAddr)->sin_addr);
	// printf("Local ip address: %s, myIP: %s\n", ip2, myIP);
	// printf("Local port: %u\n", myPort);
	fflush(stdout);

	// Transferring peer name, content name, tcp ip address, tcp port info into buf variable
	strcpy(buf, name);
	memset(buf + strlen(buf), ' ', 100 - strlen(buf) - 1);

	printf("buf: %s\n", buf);
	fflush(stdout);
	if (providedContentName == NULL)
	{
		printf("Enter content name: ");
		scanf("%s", contentName);
		strcpy(&buf[9], contentName);
	}
	else
	{
		printf("providedContentName: %s\n", providedContentName);
		strcpy(&buf[9], providedContentName);
	}

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
	int alen = sizeof(server);

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

		if (listen(tcpSock, 5) < 0)
		{
			perror("listen");
			fprintf(stderr, "Error listening to ports\n");
		}
		//(void)signal(SIGCHLD, reaper);

		int n;
		char buf2[100];
		PDU downloadPacket;
		struct sockaddr_in client;
		int client_len = sizeof(struct sockaddr_in);

		// Create a forked process to listen to TCP content server
		switch (fork())
		{
		case 0:
			while (1)
			{
				char fileBuf[BUFLEN];
				int readBufPointer = 0, display;
				PDU contentPacket;
				char *contentName;
				char *usr;
				FILE *fp;
				// Accept any new TCP connection
				new_sd = accept(tcpSock, (struct sockaddr *)&client, &client_len);

				printf("accepted new client, waiting for message, %d\n", new_sd);

				bzero(downloadPacket.data, BUFLEN);
				// Receive download packet: TYPE | user | content name
				n = recv(new_sd, &downloadPacket, sizeof(PDU), 0);
				printf("packet received from tcp client: %s\n", downloadPacket.data);

				// Parse info from download packet
				usr = strtok(downloadPacket.data, " ");
				contentName = strtok(NULL, " ");

				printf("usr: %s, contentName: %s\n", usr, contentName);
				fflush(stdout);

				contentName[strcspn(contentName, "\n")] = 0;
				fp = fopen(contentName, "r");
				if (fp == NULL)
				{
					// File does not exist
					fprintf(stderr, "Error, file does not exist.\n", 27);
					contentPacket.type = 'E';
					strcpy(contentPacket.data, "Error, file does not exist.");
					write(new_sd, &contentPacket, sizeof(PDU));
				}
				else
				{
					// File exists
					while (1)
					{
						// reading file
						display = fgetc(fp);
						contentPacket.data[readBufPointer] = display;
						readBufPointer++;

						if (readBufPointer == BUFLEN)
						{
							// send file contents
							contentPacket.type = 'C';
							printf("sending content packet: %s\n", contentPacket.data);
							fflush(stdout);
							write(new_sd, &contentPacket, sizeof(PDU));
							readBufPointer = 0;
							bzero(contentPacket.data, sizeof(contentPacket.data));
						}

						if (feof(fp) || display == EOF)
						{
							if (readBufPointer != 0)
							{
								fflush(stdout);
								contentPacket.type = 'F';
								write(new_sd, &contentPacket, sizeof(PDU));
							}
							break;
						}
					}
					fclose(fp);
					// Deregister content
					deregistration(serverSock, contentName, server);
					break;
				}
			}
			close(new_sd);
			printf("Closed tcp sock");
			break;
		case -1:
			fprintf(stderr, "Error was found while creating child process");
			break;
		}
		printRegisteredContent();
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

void printRegisteredContent()
{
	int i = 0;
	printf("\nPrinting registered content\n");
	for (i = 0; i < maxRegContent; i++)
	{
		if (registeredContent[i].tcpSock == -1)
			break;
		printf("\tContent name: %s, sock: %d\n", registeredContent[i].name, registeredContent[i].tcpSock);
	}
	return;
}