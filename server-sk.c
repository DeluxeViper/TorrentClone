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
void registration(int, PDU, struct sockaddr_in);
void deregistration(int, char *, struct sockaddr_in);
int nameExistsInList(char *, char *);
void printList();
void printListWithHead(ENTRY *);

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
	{
		strcpy(connList[n].peerName, "");
		connList[n].head = NULL;
	}

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
			registration(s, rpdu, fsin);
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
				char *ip2 = inet_ntoa(((struct sockaddr_in *)clientAddr)->sin_addr);
				printf("found clientAddr: %s, %u\n", ip2, clientAddr->sin_port);
				spdu.type = 'S';
				char address[100];
				bzero(address, sizeof(address));
				strcat(address, ip2);
				strcat(address, " ");
				sprintf(address + strlen(address), "%d", clientAddr->sin_port);
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
			deregistration(s, &rpdu.data, fsin);
		}
	}
	return;
}

void deregistration(int s, char *data, struct sockaddr_in fsin)
{
	/* De-register the server of that content
	 */
	// Search for the content within the server and remove content
	// Data = usr contentName
	char *usr, *contentName;
	PDU aPacket;

	// Parse data
	usr = strtok(data, " ");
	contentName = strtok(NULL, " ");

	printf("Unregistering peer: %s with content name: %s\n", usr, contentName);
	// Find data within linked list and remove it
	ENTRY *temp, *prev;
	int i = 0, j = 0;
	int matchFound = 1;

	for (i = 0; i < max_index; i++)
	{
		// Iterate through peers
		if (strcmp(usr, connList[i].peerName) != 0)
		{
			// If name does not match, keep going
			continue;
		}
		// Found peer, iterate through contents of peer
		temp = connList[i].head;
		printf("init curr: %s\n", temp->contentName);

		// Remove content
		if (temp != NULL && strcmp(temp->contentName, contentName) == 0)
		{
			connList[i].head = temp->next;
			free(temp);
			if (connList[i].head == NULL)
			{
				// If list is empty, deregister the entire peer
				strcpy(connList[i].peerName, "");
				max_index--;
			}
			break;
		}

		while (temp != NULL && strcmp(temp->contentName, contentName) != 0)
		{
			prev = temp;
			printf("prev:%s, curr: %s, next: %s\n", prev->contentName, temp->contentName, temp->next->contentName);
			temp = temp->next;
		}

		if (temp == NULL)
		{
			printf("Deregistration: no match found.\n");
			matchFound = 0;
			break;
		}

		prev->next = temp->next;

		free(temp);
	}
	printList();

	// Send Ack packet
	if (matchFound == 0)
	{
		// No match found
		aPacket.type = 'E';
		printf("Error with deregistration.\n");
		strcpy(aPacket.data, "Error with deregistration, no match was found");
	}
	else
	{
		// Match found
		aPacket.type = 'A';
		printf("Deregistration successful.\n");
		strcpy(aPacket.data, "Acknowledged");
	}

	sendto(s, &aPacket, sizeof(PDU), 0,
		   (struct sockaddr *)&fsin, sizeof(fsin));
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
				// printf("entry ip addr: %s", inet_ntoa(((struct sockaddr_in *)&entry->addr)->sin_addr));
				return &entry->addr;
			}
			entry = entry->next;
		}
	}
	return NULL;
}

void registration(int s, PDU rpdu, struct sockaddr_in fsin)
{
	ENTRY *curr_entry = NULL;
	/* Register the content and the server of the content
	 */
	char *tokArr[4]; // [0] = peerName, [1] = contentName, [2] = ip, [3] = port (host format)
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
		// Content was not found, prepare A packet to be sent
		printf("Name does not exist in list\n");
		fflush(stdout);
		spdu.type = 'A';
		bzero(spdu.data, BUFLEN);

		// Populate entry with address and content name value received
		ENTRY *entry = malloc(sizeof(ENTRY));
		strcpy(entry->contentName, tokArr[1]);

		entry->addr.sin_family = AF_INET;
		entry->addr.sin_port = atoi(tokArr[3]); // convert ip address back to host
		entry->addr.sin_addr.s_addr = inet_addr(tokArr[2]);

		// If peer has deleted all its deregistered content then this
		// is necessary to check
		int existingPeerIndex = -1;
		for (int k = 0; k < max_index; k++)
		{
			if (strcmp(connList[k].peerName, tokArr[0]) == 0)
			{
				existingPeerIndex = k;
				break;
			}
		}
		// If peer is registering it's first content, then add it first to the
		// 	connection list
		// Check if connList has the peerName
		if (existingPeerIndex == -1) // -1 for peerNameExists indicates that the peer does not exist
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
			printf("Peer has registered content before: %d\n", existingPeerIndex);
			// peer has registered content before, simply add content to the linkedlist
			curr_entry = entry;
			curr_entry->next = connList[existingPeerIndex].head;
			connList[existingPeerIndex].head = curr_entry;

			printList();
			printf("name exists in list: %d\n", nameExistsInList(tokArr[0], tokArr[1]));
		}
	}

	// Send SPU -> which will either be an A packet or an E packet based
	// 	on whether the content was found
	printf("spdu to send: %c, %s\n", spdu.type, spdu.data);
	sendto(s, &spdu, sizeof(PDU), 0,
		   (struct sockaddr *)&fsin, sizeof(fsin));
	printf("sent spdu\n");
	fflush(stdout);
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
			// char *ip = inet_ntop(((struct sockaddr_in *)&entry->addr)->sin_addr);
			char ip[100];
			inet_ntop(AF_INET, &(((struct sockaddr_in *)&entry->addr)->sin_addr), ip, 100);
			printf("contentName: %s, ip: %s, port: %d\n", entry->contentName, ip, entry->addr.sin_port);
			entry = entry->next;
		}
	}
	return;
}

void printListWithHead(ENTRY *head)
{
	printf("Print list with head.\n");
	ENTRY *entry = head;
	while (entry != NULL)
	{
		printf("\t\tEntry: %d ");
		// char *ip = inet_ntop(((struct sockaddr_in *)&entry->addr)->sin_addr);
		char ip[100];
		inet_ntop(AF_INET, &(((struct sockaddr_in *)&entry->addr)->sin_addr), ip, 100);
		printf("contentName: %s, ip: %s, port: %d\n", entry->contentName, ip, entry->addr.sin_port);
		entry = entry->next;
	}
}
