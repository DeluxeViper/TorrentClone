#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#define BUFSIZE 65

struct PDU {
	char type;
	char data[100];
};

struct Registration {
	char peerName[10];
	char contentName[10];
	char address[10];	
};

int main(int argc, char *argv[])
{
	char *host = "localhost";
//	char *portNum = "3000";

	switch (argc) {
		case 1:
			break;	
		case 3:
			break;
		case 2:
			break;
		default:
			fprintf(stderr, "usage: peer [host [port]]\n");
			exit(1);	
	}

	// Reset the value of sin
//	memset(&server, 0, sizeof(server));
//	server.sin_family = AF_INET;

	registerContent(host, 3000);	
}

// Content registration via UDP
void registerContent(char* SERVER_ADDR, int SERVER_PORT) {
	struct PDU registerContentPacket;
	struct Registration regis;

	struct sockaddr_in serverAddr, myAddr;
	struct hostent *hp;
	int sock;
	int myIP;
	unsigned int myPort;

	// Connect to server
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Cannot open socket");
		exit(1);
	}

	// Set server addr
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	serverAddr.sin_port = htons(SERVER_PORT);

	if (hp = gethostbyname(SERVER_ADDR)) {
		bcopy(hp->h_addr, (char *)&serverAddr.sin_addr, hp->h_length);
	} else if (inet_aton(SERVER_ADDR, (struct in_addr *) &serverAddr.sin_addr)) {
		fprintf(stderr, "Cannot get server's address\n");
		exit(1);
	}
	char * ip = inet_ntoa(((struct sockaddr_in *)&serverAddr)->sin_addr);
	// Connect to server
	if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		printf("Attempted to connect to %s, %d \n", ip, serverAddr.sin_port);	
		fprintf(stderr, "Error connecting to index server");
		close(sock);
		exit(-1);
	}

	// Get my ip address and port
	bzero(&myAddr, sizeof(myAddr));
	socklen_t len = sizeof(myAddr);
	getsockname(sock, (struct sockaddr *) &myAddr, &len);
	inet_ntop(AF_INET, &myAddr.sin_addr, myIP, sizeof(myIP));

	myPort = ntohs(myAddr.sin_port);

	printf("Local ip address: %s\n", myIP);
	printf("Local port: %u\n", myPort);

//	printf("Enter peer name: ");
//	scanf("%s", &regis.peerName);

//	printf("Enter content name: ");
//	scanf("%s", &regis.contentName); 
}

// Request content from index server via UDP


// Download content via TCP
