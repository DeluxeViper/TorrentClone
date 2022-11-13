#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
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

int main(int argc, char *argv[])
{
    // Address of a client
    struct sockaddr_in clientAddress;

    // Internet endpoint address
    struct sockaddr_in serverAddress;

    int port; // port of server
    int sock; // socket that the server opens
    int alen; // address length
    char buf[100];
    struct PDU pduData;

    switch (argc)
    {
    case 1:
        printf("case 1\n");
        break;
    case 2:
        port = atoi(argv[1]);
        break;
    default:
        fprintf(stderr, "Usage %s [port]\n", argv[0]);
        exit(1);
    }

    // memset(&serverAddress, 0, sizeof(serverAddress));
    bzero((char *)&serverAddress, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
    // serverAddress.sin_addr.s_addr = inet_addr("10.0.0.251");
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // Allocate a socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        fprintf(stderr, "cannot create socket\n");

    // Bind the socket
    if (bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "cannot bind to %d port\n", port);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Bound to socket\n");
        fflush(stdout);
    }

    // Queue up to 5 connect requests
    listen(sock, 5);
    alen = sizeof(clientAddress);

    while (1)
    {
        if (recvfrom(sock, &pduData, sizeof(struct PDU), 0, (struct sockaddr *)&clientAddress, &alen) < 0)
        {
            printf("pduData: %s\n", &pduData);
            fflush(stdout);
            fprintf(stderr, "recvfrom err\n");
            return -1;
        }
        else
        {
            // buf[strcspn(buf, "\n")] = 0;
            printf("Received: %s\n", &pduData);
            fflush(stdout);
        }

        sendto(sock, &pduData, sizeof(struct PDU), 0, (struct sockaddr *)&clientAddress, alen);
        printf("message sent\n");
        fflush(stdout);
    }
}