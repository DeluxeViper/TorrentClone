
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdlib.h>

#define BUFSIZE 65
#define NUM_SOCKETS 3

typedef struct PDU
{
	char type;
	char data[100];
} PDU;

typedef struct Address
{
	char *ipAddr;
	int port;
} Address;

void registerContent(char *SERVER_ADDR, int SERVER_PORT, struct Address tcpAddr);
struct Address createTcpSocket(struct sockaddr_in tcpAddr, int tcpSock);
void requestContentFromServer();
void downloadContentFromPeer();

int main(int argc, char *argv[])
{
	char *host = "10.0.0.251";
	int port = 15050;
	//	char *portNum = "3000";
	int tcpSockets[NUM_SOCKETS];
	int currTcpSock = 0;
	// struct Address addr = malloc(sizeof(struct Address));
	struct Address addr;
	struct sockaddr_in tcpAddrs[NUM_SOCKETS];

	switch (argc)
	{
	case 1:
		break;
	case 3:
		// Host
		host = argv[1];
	case 2:
		// Only port
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "usage: peer [host [port]]\n");
		exit(1);
	}
	// Create the tcp socket
	addr = (struct Address)createTcpSocket(tcpAddrs[currTcpSock], tcpSockets[currTcpSock]);
	printf("received addr: %s\n", addr);
	fflush(stdout);
	registerContent(host, port, (struct Address)addr);
	currTcpSock++;
}

/**
 * @brief
 * @param SERVER_ADDR - address of the index server (UDP)
 * @param SERVER_PORT - the index server port (UDP)
 * @return * Content - Returns an integer that indicates whether registering content was successful
 */
void registerContent(char *SERVER_ADDR, int SERVER_PORT, struct Address tcpAddr)
{
	struct PDU registerPacket;
	struct sockaddr_in serverAddr;

	// TCP socket opened by peer
	struct sockaddr_in tcpSock;
	// pointer to host information entry
	struct hostent *hp;
	int udpSock;
	struct PDU received;
	int n;
	int alen;
	char buf[100];

	printf("port: %d\n", SERVER_PORT);
	fflush(stdout);
	// Connect to server
	if ((udpSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf(stderr, "Cannot open socket");
		exit(1);
	}

	// Set server addr
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	// serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	serverAddr.sin_port = htons((u_short)SERVER_PORT);

	if (hp = gethostbyname(SERVER_ADDR))
	{
		bcopy(hp->h_addr, (char *)&serverAddr.sin_addr, hp->h_length);
	}
	else if ((serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR)) == INADDR_NONE)
	{
		fprintf(stderr, "Cannot get server's address\n");
		exit(1);
	}

	char *ip = inet_ntoa(((struct sockaddr_in *)&serverAddr)->sin_addr);

	// Connect to server
	if (connect(udpSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{

		printf("Attempted to connect to %s, %s \n", ip, ntohs(serverAddr.sin_port));
		fprintf(stderr, "Error connecting to index server\n");
		close(udpSock);
		exit(0);
	}
	else
	{
		printf("Connected to server\n");
		fflush(stdout);
	}
	printf("Enter peer name: ");
	scanf("%s", &buf[0]);
	printf("buf: %s\n", &buf);
	fflush(stdout);
	memset(buf + strlen(buf), ' ', 100 - strlen(buf) - 1);
	printf("Enter content name: ");
	scanf("%s", &buf[9]);
	memset(buf + strlen(buf), ' ', 100 - strlen(buf) - 1);
	strcpy(buf + 19, tcpAddr.ipAddr);
	memset(buf + strlen(buf), ' ', 100 - strlen(buf) - 1);
	sprintf(buf + 19 + strlen(tcpAddr.ipAddr) + 1, "%d", tcpAddr.port);

	// Set regis address to tcpAddr
	//	printf("Setting regis address to tcp Address");

	fflush(stdout);
	buf[99] = '\0';
	strcpy(&registerPacket.data, &buf);
	registerPacket.type = 'R';
	printf("registerContentPacket data: %s\n", &registerPacket);
	fflush(stdout);
	write(udpSock, &registerPacket, sizeof(struct PDU));
	printf("wrote message\n");
	fflush(stdout);

	// n = recvfrom(sock, received, sizeof(received), MSG_WAITALL, (struct sockaddr_in *)&serverAddr, &alen);
	n = recvfrom(udpSock, &received, sizeof(struct PDU), MSG_WAITALL, (struct sockaddr *)&serverAddr, &alen);
	if (n < 0)
		fprintf(stderr, "Read failed\n");

	printf("received: %s\n", &received);
	fflush(stdout);

	close(udpSock);
	exit(0);
}

// Creates and opens a TCP socket
// Returns an address for the created socket
struct Address createTcpSocket(struct sockaddr_in tcpAddr, int tcpSock)
{
	unsigned int myPort;
	int myIP;
	struct Address addr;
	// Get my ip address and port
	bzero(&tcpAddr, sizeof(tcpAddr));
	socklen_t len = sizeof(tcpAddr);
	tcpSock = socket(AF_INET, SOCK_STREAM, 0);
	tcpAddr.sin_family = AF_INET;
	tcpAddr.sin_port = htons(0);
	tcpAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(tcpSock, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr));

	getsockname(tcpSock, (struct sockaddr *)&tcpAddr, &len);
	inet_ntop(AF_INET, &tcpAddr.sin_addr, myIP, sizeof(myIP));

	myPort = ntohs(tcpAddr.sin_port);
	char *ip2 = inet_ntoa(((struct sockaddr_in *)&tcpAddr)->sin_addr);
	printf("Local ip address: %s\n", ip2);
	printf("Local port: %u\n", myPort);
	fflush(stdout);

	strcpy(&addr.ipAddr, &ip2);

	addr.port = myPort;

	return addr;
}

// Request content from index server via UDP
void requestContentFromServer()
{
}

// Download content via TCP
void downloadContentFromPeer()
{
}

void printCharArray(char *buf)
{
	int i = 0;
	for (i = 0; i < 100; i++)
	{
		printf("%c, ", buf[i]);
	}
	printf("\n");
}