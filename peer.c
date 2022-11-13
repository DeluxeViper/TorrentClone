
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#define BUFSIZE 65

struct PDU
{
	char type;
	char data[100];
};

struct Registration
{
	char peerName[10];
	char contentName[10];
	char address[10];
};

int main(int argc, char *argv[])
{
	char *host = "10.0.0.251";
	int port = 15050;
	//	char *portNum = "3000";

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

	// Reset the value of sin
	//	memset(&server, 0, sizeof(server));
	//	server.sin_family = AF_INET;

	registerContent(host, port);
}

// Content registration via UDP
void registerContent(char *SERVER_ADDR, int SERVER_PORT)
{
	struct PDU registerContentPacket;
	struct Registration regis;

	struct sockaddr_in serverAddr, myAddr;
	// pointer to host information entry
	struct hostent *hp;
	int sock;
	int myIP;
	unsigned int myPort;
	struct PDU received;
	int n;
	int alen;

	printf("port: %d\n", SERVER_PORT);
	fflush(stdout);
	// Connect to server
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
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
	if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{

		printf("Attempted to connect to %s, %s \n", ip, ntohs(serverAddr.sin_port));
		fprintf(stderr, "Error connecting to index server\n");
		close(sock);
		exit(-1);
	}
	else
	{
		printf("Connected to server\n");
		fflush(stdout);
	}

	// Get my ip address and port
	bzero(&myAddr, sizeof(myAddr));
	socklen_t len = sizeof(myAddr);
	getsockname(sock, (struct sockaddr *)&myAddr, &len);
	inet_ntop(AF_INET, &myAddr.sin_addr, myIP, sizeof(myIP));

	myPort = ntohs(myAddr.sin_port);
	char *ip2 = inet_ntoa(((struct sockaddr_in *)&myAddr)->sin_addr);
	printf("Local ip address: %s\n", ip2);
	printf("Local port: %u\n", myPort);
	fflush(stdout);

	strcpy(registerContentPacket.data, "hola123");
	registerContentPacket.type = 'R';
	write(sock, &registerContentPacket, sizeof(struct PDU));
	// write(sock, &registerContentPacket, sizeof(struct PDU));
	printf("wrote message\n");
	fflush(stdout);

	// n = recvfrom(sock, received, sizeof(received), MSG_WAITALL, (struct sockaddr_in *)&serverAddr, &alen);
	n = recvfrom(sock, &received, sizeof(struct PDU), MSG_WAITALL, (struct sockaddr *)&serverAddr, &alen);
	if (n < 0)
		fprintf(stderr, "Read failed\n");

	printf("received: %s\n", &received);
	fflush(stdout);

	close(sock);
	exit(0);

	//	printf("Enter peer name: ");
	//	scanf("%s", &regis.peerName);

	//	printf("Enter content name: ");
	//	scanf("%s", &regis.contentName);
}

// Request content from index server via UDP
void requestContentFromServer()
{
}

// Download content via TCP
void downloadContentFromPeer()
{
}