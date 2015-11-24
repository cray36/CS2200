#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rtp.h"

/* GIVEN Function:
 * Handles creating the client's socket and determining the correct
 * information to communicate to the remote server
 */
CONN_INFO* setup_socket(char* ip, char* port){
	struct addrinfo *connections, *conn = NULL;
	struct addrinfo info;
	memset(&info, 0, sizeof(struct addrinfo));
	int sock = 0;

	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_DGRAM;
	info.ai_protocol = IPPROTO_UDP;
	getaddrinfo(ip, port, &info, &connections);

	/*for loop to determine corr addr info*/
	for(conn = connections; conn != NULL; conn = conn->ai_next){
		sock = socket(conn->ai_family, conn->ai_socktype, conn->ai_protocol);
		if(sock <0){
			if(DEBUG)
				perror("Failed to create socket\n");
			continue;
		}
		if(DEBUG)
			printf("Created a socket to use.\n");
		break;
	}
	if(conn == NULL){
		perror("Failed to find and bind a socket\n");
		return NULL;
	}
	CONN_INFO* conn_info = malloc(sizeof(CONN_INFO));
	conn_info->socket = sock;
	conn_info->remote_addr = conn->ai_addr;
	conn_info->addrlen = conn->ai_addrlen;
	return conn_info;
}

void shutdown_socket(CONN_INFO *connection){
	if(connection)
		close(connection->socket);
}

/* 
 * ===========================================================================
 *
 *			STUDENT CODE STARTS HERE. PLEASE COMPLETE ALL FIXMES
 *
 * ===========================================================================
 */


/*
 *  Returns a number computed based on the data in the buffer.
 */
static int checksum(char *buffer, int length){
    int sum = 0;
    for ( int i = 0; i < length; i++ ) {
        sum += (int) buffer[i];
    }
    return sum;

	/*  ----  FIXME  ----
	 *
	 *  The goal is to return a number that is determined by the contents
	 *  of the buffer passed in as a parameter.  There a multitude of ways
	 *  to implement this function.  For simplicity, simply sum the ascii
	 *  values of all the characters in the buffer, and return the total.
	 */ 
}

/*
 *  Converts the given buffer into an array of PACKETs and returns
 *  the array.  The value of (*count) should be updated so that it 
 *  contains the length of the array created.
 */
static PACKET* packetize(char *buffer, int length, int *count){

	/*  ----  FIXME  ----
	 *  The goal is to turn the buffer into an array of packets.
	 *  You should allocate the space for an array of packets and
	 *  return a pointer to the first element in that array.  Each 
	 *  packet's type should be set to DATA except the last, as it 
	 *  should be LAST_DATA type. The integer pointed to by 'count' 
	 *  should be updated to indicate the number of packets in the 
	 *  array.
	 */
    PACKET* packets;
    PACKET* packet;
    int count_packets, i;
    count_packets = length / MAX_PAYLOAD_LENGTH;
    /*if the last packet size is not zero, use another packet*/
    if (length % MAX_PAYLOAD_LENGTH)
        count_packets++;
    *count = count_packets;
    /*Allocate memory for array of packets*/
    packets = (PACKET*) calloc(count_packets, sizeof(PACKET));
    if (packets != NULL)
    {
        for (i=0; i<count_packets; i++) {
            packet = packets+i;
            /*default packet type*/
            packet->type = DATA;
            
            /*default packet length*/
            packet->payload_length = MAX_PAYLOAD_LENGTH;
            
            /*if last packet*/
            if (i == count_packets - 1)
            {
                /*change packet's type to LAST_DATA*/
                packet->type = LAST_DATA;
                size_t remaining_size = length % MAX_PAYLOAD_LENGTH;
                /*if 0 then length divides payload_length*/
                if (remaining_size == 0)
                    remaining_size = MAX_PAYLOAD_LENGTH;
                
                /*update correct payload_length*/
                packet->payload_length = remaining_size;
            }
            
            /*copy data from buffer to packet's payload*/
            memcpy(packet->payload, buffer + i * MAX_PAYLOAD_LENGTH, packet->payload_length);
            
            /*calculate checksum*/
            packet->checksum = checksum(packet->payload, packet->payload_length);
        }
    }
    return packets;

}

/*
 * Send a message via RTP using the connection information
 * given on UDP socket functions sendto() and recvfrom()
 */
int rtp_send_message(CONN_INFO *connection, MESSAGE*msg){
	/* ---- FIXME ----
	 * The goal of this function is to turn the message buffer
	 * into packets and then, using stop-n-wait RTP protocol,
	 * send the packets and re-send if the response is a NACK.
	 * If the response is an ACK, then you may send the next one
	 */
	 int* count = (int*)malloc(sizeof(int));
	 *count = 0;
	 PACKET* packet = packetize(msg -> buffer, msg -> length, count);
	 int size = *count;

	 int socketfd = connection -> socket;
	 struct sockaddr* src_addr = connection -> remote_addr;
	 socklen_t addrlen = connection -> addrlen;
	 socklen_t* paddrlen = &(addrlen);

	 PACKET* receive = (PACKET*)malloc(sizeof(PACKET));
	 while (1) {
	 	sendto(socketfd, packet, sizeof(PACKET), 0, src_addr, addrlen);
	 	recvfrom(socketfd, receive, sizeof(PACKET), 0, src_addr, paddrlen);
	 	if (receive -> type == ACK) {
	 		packet = packet + 1;
	 		size--;
	 		if (size == 0) {
	 			break;
	 		}
	 	} else if (receive -> type != ACK && receive -> type != NACK) {
	 		return -1;
	 	}
	 }

	 return 0;

}

/*
 * Receive a message via RTP using the connection information
 * given on UDP socket functions sendto() and recvfrom()
 */
MESSAGE* rtp_receive_message(CONN_INFO *connection){
	/* ---- FIXME ----
	 * The goal of this function is to handle 
	 * receiving a message from the remote server using
	 * recvfrom and the connection info given. You must 
	 * dynamically resize a buffer as you receive a packet
	 * and only add it to the message if the data is considered
	 * valid. The function should return the full message, so it
	 * must continue receiving packets and sending response 
	 * ACK/NACK packets until a LAST_DATA packet is successfully 
	 * received.
	 */
	 MESSAGE* message = (MESSAGE*)malloc(sizeof(MESSAGE));
	 message -> length = 0;
	 PACKET* packet = (PACKET*)malloc(sizeof(PACKET));
	
	/*
	* we need a counter to terminate the the message, so we can use the
	* last byte of each message as the null terminate. So we need to 
	* initilize this.
	*/
	 char* message_buffer = (char*)malloc(sizeof(char));

	 int socketfd = connection -> socket;
	 struct sockaddr* src_addr = connection -> remote_addr;
	 socklen_t addrlen = connection -> addrlen;
	 socklen_t* paddrlen = &(addrlen);
	 int current_position = 0;

	 while (1) {
	 	recvfrom(socketfd, packet, sizeof(PACKET), 0, src_addr, paddrlen);
		
		if (checksum(packet -> payload, packet -> payload_length) == packet -> checksum) {
			message_buffer = (char*)realloc(message_buffer, ((packet -> payload_length) + (message -> length)) * sizeof(char));
			for (int i = 0; i < packet -> payload_length; i++) {
				message_buffer[current_position] = (packet -> payload)[i];
				current_position++;
			}

			message -> length = message -> length + packet -> payload_length;
			PACKET* ack_packet = (PACKET*)malloc(sizeof(PACKET));
			ack_packet -> type = ACK;
			sendto(socketfd, ack_packet, sizeof(PACKET), 0, src_addr, addrlen);
			if (packet -> type == LAST_DATA) {
				message_buffer[current_position] = '\0';
				break;
			}
		} 
		else {
			PACKET* nack_packets = (PACKET*)malloc(sizeof(PACKET));
			nack_packets -> type = NACK;
			sendto(socketfd, nack_packets, sizeof(PACKET), 0, src_addr, addrlen);
		}
	 	
	 }
	 message -> buffer = message_buffer;
	 return message;
}
