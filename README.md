# TorrentClone

## TLDR
A P2P application (created in C) that allows peers to share and download content through an index server via UDP and TCP communication. 

## Technical Details

This torrent clone project consists of a P2P (Peer-to-Peer) application that allows clients to share and download content through an index server utilizing UDP for communication and TCP for content download. The peers act as clients to upload and download content and act as servers to allow other peers to view and download content, making the peer both a client and server for content. The index server facilitates these activities allowing the peers to exchange content amongst each other.

![Socket Diagram](https://user-images.githubusercontent.com/60635737/234313166-d3d899fd-8b45-4393-8e26-ec0522749440.png)

Figure 1: Socket Diagram

The peer to peer application consists of TCP and UDP socket connections. Both socket protocols can be used for all elements of this project, however the performance and reliability of each vary depending on the task. UDP is better suited for handling the peer connections as it is a connectionless service that allows multiple users to connect simultaneously. There is no acknowledgement sent with each message in UDP, which allows for quicker connections than TCP. However, UDP lacks in lossless data transfer as the acknowledgement messages in TCP connection protocol ensure no data is lost during transmission. As shown in the figure above, S1, S5, S6, S8 use TCP sockets for content download and S2, S3, S4, S7 use UDP sockets for index server and peer connections. Thus, UDP is used for peer to peer communication and TCP is used to handle content exchange.

![UDP Connection](https://user-images.githubusercontent.com/60635737/234313442-c8099a7e-b5b7-4339-8515-a6bbee366fd8.png)

Figure 2: UDP Connection

The peer first establishes a connection with the index server using UDP socket connection allowing peers to become clients and communicate along the index server. To register a file, the peer will connect to the index server including the IP address of the peer and the socket port number alongside the content upload. These will be used by other peers to reference content on the server and allow registering and deregistering of content.

## Peer description
**General Code Description & Implementation method:**

The client program was implemented using a base while loop that checked the commands that were being received. The client program retained a global “registeredContent” list to keep track of content that was being registered to the index server, as well as the corresponding TCP socket to allow the client to pose as a content server when certain content was being requested. Once this was established, it was only a matter of creating specific functions – to which their functions and descriptions are outlined below this paragraph – to meet the requirements of the client.  

**Table of PDU type with their corresponding function & descriptions:**


<table>
  <tr>
   <td>PDU Type
   </td>
   <td>Function
   </td>
   <td>Direction
   </td>
  </tr>
  <tr>
   <td>R
   </td>
   <td>
<ul>

<li>Accepts download requests by starting a passive TCP socket

<li>Peer that acts as the content server registers its content and sends an R type PDU with peer name, content name, and port address to the index server

<li>The address that is registered becomes the port number of the newly created TCP socket

<li>If the peer is downloading content, sends an R type PDU to register the downloaded content and becomes the new server
</li>
</ul>
   </td>
   <td>Sending: peer to index server
   </td>
  </tr>
  <tr>
   <td>D
   </td>
   <td>
<ul>

<li>TCP connection established with the content server and the address extracted with the S type PDU
</li>
</ul>
   </td>
   <td>Peer (content server) to peer (content client)
   </td>
  </tr>
  <tr>
   <td>S
   </td>
   <td>
<ul>

<li>Contacts the index server to request the address of the content server that holds the requested content with an S type PDU containing peer name and content name

<li>Extracts the address from the S type PDU that’s sent from the index server
</li>
</ul>
   </td>
   <td>Sending: peer (content server) to index server
<p>
Receiving: index server to peer (content client)
   </td>
  </tr>
  <tr>
   <td>C
   </td>
   <td>
<ul>

<li>Transmits the requested content through the TCP connection

<li>If the content is larger than 1000 bytes, the file will be sent through multiple packets

<li>Content server terminates upon completion of the download
</li>
</ul>
   </td>
   <td>Sending: peer (content server) to peer (content client)
   </td>
  </tr>
  <tr>
   <td>T
   </td>
   <td>
<ul>

<li>Peer makes a request to deregister its content from the server
</li>
</ul>
   </td>
   <td>Peer (content server) to index server)
   </td>
  </tr>
  <tr>
   <td>Q
   </td>
   <td>
<ul>

<li>Peer makes a request to quit and deregister all its contents through T type PDUs
</li>
</ul>
   </td>
   <td>Peer (content server) to index server
   </td>
  </tr>
  <tr>
   <td>O
   </td>
   <td>
<ul>

<li>Lists all registered content in the server upon user request
</li>
</ul>
   </td>
   <td>Sending: peer to index server
<p>
Receiving: index server to peer
   </td>
  </tr>
</table>


## Server Program Description
**General Code Description & Implementation method:**

The server was implemented using a while loop in the main method that waited for specific messages to be received from the client (via UDP), and based on the PDU type, executed certain functions. The server contains a global variable to retain the list of connections being made, and each connection structure would contain the peer name and a corresponding content linked list, with each entry depicting the content name, address, port, and a pointer to the next linked list entry. The table below outlines the pseudocode for each PDU type received by the server.

**Table of PDU type with their corresponding function & descriptions:**


<table>
  <tr>
   <td>PDU Type
   </td>
   <td>Function
   </td>
   <td>Direction
   </td>
  </tr>
  <tr>
   <td>R
   </td>
   <td>
<ul>

<li>Performs a check to see if another peer with the same name has already registered content with the same file name

<li>If no conflicts in user and content names exist, content is registered and associated with the address of the content server
</li>
</ul>
   </td>
   <td>Receiving: peer to index server
<p>
Peer becomes content server after registration
   </td>
  </tr>
  <tr>
   <td>S
   </td>
   <td>
<ul>

<li>Searches of the address of the content server that the peer requests to download from and sends the address if found

<li>Ensures that the address of the content server with the least interaction is sent
</li>
</ul>
   </td>
   <td>Sending: peer (content client) to index server
<p>
Receiving: index server to peer (content client)
   </td>
  </tr>
  <tr>
   <td>T
   </td>
   <td>
<ul>

<li>Deregisters contents if requested by the peer
</li>
</ul>
   </td>
   <td>Peer (content server) to index server
   </td>
  </tr>
  <tr>
   <td>O
   </td>
   <td>
<ul>

<li>Sends a list of all registered contents to the peer
</li>
</ul>
   </td>
   <td>Sending: peer to index server
<p>
Receiving: index server to peer
   </td>
  </tr>
  <tr>
   <td>A
   </td>
   <td>
<ul>

<li>Sends an acknowledgement of successful registration to the peer

<li>Sends an acknowledgement of successful deregistration to the peer
</li>
</ul>
   </td>
   <td>Index server to peer
   </td>
  </tr>
  <tr>
   <td>E
   </td>
   <td>
<ul>

<li>Sends an error during content registration if two peers exist with the same name and content name

<li>Sends an error when the search for an address of the desired content fails
</li>
</ul>
   </td>
   <td>Index server to peer
   </td>
  </tr>
</table>
