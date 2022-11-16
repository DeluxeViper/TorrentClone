/* Index Server

Message types:
R - used for registration
A - used by the server to acknowledge the success of registration
Q - used by chat users for de-registration
D - download content between peers (not used here)
C - Content (not used here)
S - Search content
L - Location of the content server peer
E - Error messages from the Server

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>

#define ERRMSG "Content name already exists"
#define CONTENT_NOT_FOUND "Cannot find content from other peers"
#define BUFLEN 100
#define NAMESIZ 20
#define MAX_NUM_CON 200

typedef struct entry
{
	char contentName[NAMESIZ];
	struct sockaddr_in addr;
	short token;
	struct entry *next;
} ENTRY;

typedef struct
{
	char peerName[NAMESIZ];
	ENTRY *head;
} LIST;

LIST connList[MAX_NUM_CON];

int max_index = 0;

typedef struct
{
	char type;
	char data[BUFLEN];
} PDU;

PDU tpdu;

struct sockaddr_in *search(int, char *, char *);
void registration(int, PDU, struct sockaddr_in, ENTRY *);
void deregistration(int, char *, struct sockaddr_in *);
int nameExistsInList(char *, char *);
void printList();

/*
 *------------------------------------------------------------------------
 * main - Iterative UDP server for Content Indexing service
 *------------------------------------------------------------------------
 */

int main(int argc, char *argv[])
{
	struct sockaddr_in sin, *p_addr; /* the from address of a client	*/
	ENTRY *curr_entry = NULL;
	char *service = "10000"; /* service name or port number	*/
	char name[NAMESIZ], usr[NAMESIZ];
	int alen = sizeof(struct sockaddr_in); /* from-address length		*/
	int s, n, i, len, p_sock;			   /* socket descriptor and socket type    */
	int pdulen = sizeof(PDU);
	struct hostent *hp;
	PDU rpdu, spdu;
	struct sockaddr_in fsin; /* the from address of a client	*/

	for (n = 0; n < MAX_NUM_CON; n++)
		connList[n].head = NULL;

	switch (argc)
	{
	case 1:
		break;
	case 2:
		service = argv[1];
		break;
	default:
		fprintf(stderr, "usage: server-sk [host [port]]\n");
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	/* Map service name to port number */
	sin.sin_port = htons((u_short)atoi(service));

	/* Allocate a socket */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		fprintf(stderr, "can't creat socket\n");
		exit(1);
	}

	/* Bind the socket */
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %s port\n", service);

	while (1)
	{
		fflush(stdout);
		if ((n = recvfrom(s, &rpdu, pdulen, 0, (struct sockaddr *)&fsin, &alen)) < 0)
		{
			fprintf(stderr, "recvfrom error: n=%d\n", n);
		}
		/*	Content Registration Request			*/
		if (rpdu.type == 'R')
		{

			// If contained then send the E packet
			/*	Call registration()
			 */
			registration(s, rpdu, fsin, curr_entry);
		}

		/* Search Content		*/
		if (rpdu.type == 'S')
		{
			// Note: rpdu.data = <peerName> <contentName>
			printf("Searching content:\n");
			fflush(stdout);
			struct sockaddr_in *clientAddr = NULL;

			// Parsing peer name and content name from rpdu.data
			char *peerName, *contentName;
			peerName = strtok(rpdu.data, " ");
			contentName = strtok(NULL, " ");

			printf("parsed rpdu data: %s, %s\n", peerName, contentName);
			fflush(stdout);
			clientAddr = search(s, contentName, peerName);
			if (clientAddr != NULL)
			{
				char *ip2 = inet_ntoa(((struct sockaddr_in *)&clientAddr)->sin_addr);
				printf("found clientAddr: %s, %u\n", ip2, ntohs(clientAddr->sin_port));
				spdu.type = 'S';
				char address[100];
				bzero(address, sizeof(address));
				strcat(address, ip2);
				strcat(address, " ");
				sprintf(address + strlen(address), "%d", ntohs(clientAddr->sin_port));
				// strcat(address, ntohs(clientAddr->sin_port));
				printf("address to send: %s\n", address);

				strcpy(spdu.data, address);
				sendto(s, &spdu, sizeof(PDU), 0,
					   (struct sockaddr *)&fsin, sizeof(fsin));
				printf("sent spdu\n");
				fflush(stdout);
			}
			else
			{
				printf("Content not found\n");
				spdu.type = 'E';
				strcpy(spdu.data, CONTENT_NOT_FOUND);
				sendto(s, &spdu, sizeof(PDU), 0,
					   (struct sockaddr *)&fsin, sizeof(fsin));
				printf("sent spdu\n");
				fflush(stdout);
			}
		}

		/*	List current Content */
		if (rpdu.type == 'O')
		{
			/* Read from the content list and send the list to the
			   client 		*/
		}

		/*	De-registration		*/
		if (rpdu.type == 'T')
		{
			/* Call deregistration()
			 */
		}
	}
	return;
}

struct sockaddr_in *search(int s, char *contentName, char *peerName)
{
	/* Search content list and return the answer:
	   If found, send the address of the selected content server.
	*/
	ENTRY *entry;
	int i = 0;

	for (i = 0; i < max_index; i++)
	{
		if (strcmp(peerName, connList[i].peerName) == 0)
		{
			continue;
		}
		// Iterate through the linked list to check if content name exists
		entry = connList[i].head;
		while (entry != NULL)
		{
			// Check for content name match
			if (strcmp(entry->contentName, contentName) == 0)
			{
				// printf("entry content name: %s, contentName comparing: %s\n", entry->contentName, contentName);
				return &entry->addr;
			}
			entry = entry->next;
		}
	}

	return NULL;
}

void deregistration(int s, char *data, struct sockaddr_in *addr)
{
	/* De-register the server of that content
	 */
}

void registration(int s, PDU rpdu, struct sockaddr_in fsin, ENTRY *curr_entry)
{
	/* Register the content and the server of the content
	 */
	char *tokArr[4]; // [0] = peerName, [1] = contentName, [2] = ip, [3] = port
	int i = 0;
	PDU spdu;

	// Populate array that contains registration info (see tokArr comment for details)
	tokArr[i] = strtok(rpdu.data, " ");
	while (tokArr[i] != NULL)
		tokArr[++i] = strtok(NULL, " ");

	printf("Received registration packet\n");
	//  printf("peerName: %d, contentName: %d, ip: %d, port: %d", peerName, contentName, ip, port);
	printf("tokArr: %s, %s, %s, %s\n", tokArr[0], tokArr[1], tokArr[2], tokArr[3]);
	fflush(stdout);

	// Check if connList contains the name of the peer is within the list
	if (nameExistsInList(tokArr[0], tokArr[1]) == 1)
	{
		printf("NAME EXISTS IN LIST");
		fflush(stdout);
		spdu.type = 'E';
		strcpy(spdu.data, ERRMSG);

		// SEND ERROR TO PEER
		sendto(s, &spdu, sizeof(PDU), 0,
			   (struct sockaddr *)&fsin, sizeof(fsin));
	}
	else
	{
		printf("Name does not exist in list\n");
		fflush(stdout);
		spdu.type = 'A';
		bzero(spdu.data, BUFLEN);
		strcpy(spdu.data, "123");
		ENTRY *entry = malloc(sizeof(ENTRY));
		strcpy(entry->contentName, tokArr[1]);
		fflush(stdout);

		entry->addr.sin_family = AF_INET;
		entry->addr.sin_port = htons(tokArr[3]);
		entry->addr.sin_addr.s_addr = htonl(tokArr[2]); // TODO: double check this logic here

		if (connList[max_index].head == NULL)
		{
			strcpy(connList[max_index].peerName, tokArr[0]);
			printf("Entries are null for peer: %s\n", tokArr[0]);
			connList[max_index].head = entry;
			printf("entry head content name: %s\n", &connList[max_index].head->contentName);
			fflush(stdout);

			max_index++;
			printList();
		}
		else
		{
			// Iterate through linked list until NULL is met
			curr_entry = connList[max_index].head;
			while (curr_entry != NULL)
			{
				curr_entry = curr_entry->next;
			}
			curr_entry = entry;

			printList();
		}

		printf("spdu to send: %c, %s\n", spdu.type, spdu.data);
		struct sockaddr_in destAddr;
		sendto(s, &spdu, sizeof(PDU), 0,
			   (struct sockaddr *)&fsin, sizeof(fsin));
		printf("sent spdu\n");
		fflush(stdout);
	}
}

// If found then return 1, if not found then return 0
int nameExistsInList(char *peerName, char *contentName)
{
	printf("\n\nSearching whether name exists in list\n");
	ENTRY *entry;
	int i = 0;

	for (i = 0; i < max_index; i++)
	{
		// printf("comparing peerNames: %s, %s\n", connList[i].peerName, peerName);
		if (strcmp(connList[i].peerName, peerName) == 0)
		{
			// Iterate through the linked list to check if content name exists
			entry = connList[i].head;
			while (entry != NULL)
			{
				// Check for content name match
				if (strcmp(entry->contentName, contentName) == 0)
				{
					// printf("entry content name: %s, contentName comparing: %s\n", entry->contentName, contentName);
					return 1;
				}
				entry = entry->next;
			}
		}
	}
	return 0;
}

void printList()
{
	ENTRY *entry;
	int i = 0, j = 0;
	printf("Printing ConnList\n");
	for (i = 0; i < max_index; i++)
	{
		printf("\tpeerName: %s\n", connList[i].peerName);
		// print entries:
		entry = connList[i].head;
		while (entry != NULL)
		{
			printf("\t\tEntry: %d ", j++);
			printf("contentName: %s\n", entry->contentName);
			entry = entry->next;
		}
	}
	return;
}
