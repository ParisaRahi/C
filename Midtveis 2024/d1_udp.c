/* ======================================================================
 * YOU ARE EXPECTED TO MODIFY THIS FILE.
 * ====================================================================== */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  
#include <netinet/in.h>  
#include <errno.h>  

#include "d1_udp.h"

 

uint16_t calculate_checksum(uint8_t* packet, size_t packet_size, size_t data_size) {
    uint16_t checksum_odd = 0;
    uint16_t checksum_even = 0;

    packet[2] = 0;
    packet[3] = 0;

    for (size_t i = 0; i < packet_size; i++) {
        if ((i < sizeof(struct D1Header) || i >= sizeof(struct D1Header) + data_size) && (i == 2 || i == 3)) {
            continue;  
        }
        if (i % 2 == 0) {
            checksum_even ^= packet[i];
        } else {
            checksum_odd ^= packet[i];
        }
    }

    
    if (data_size % 2 != 0) {
        checksum_even ^= 0x00; 
    }

    return (checksum_even << 8) | checksum_odd;
}

D1Peer* d1_create_client() {
    D1Peer* client = malloc(sizeof(D1Peer));
    if (!client) {
        perror("Feil for å allokere D1Peer");
        return NULL;
    }

    client->socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->socket == -1) {
        perror("Feil for å opprette UDP socket");
        free(client);
        return NULL;
    }

    client->next_seqno = 0;   
    memset(&client->addr, 0, sizeof(client->addr));  

    return client;
}


D1Peer* d1_delete(D1Peer* client) {
    if (client != NULL) {
        close(client->socket);
        free(client);
    }
    return NULL;
}


int d1_get_peer_info(D1Peer* peer, const char* servername, uint16_t server_port) {
    struct hostent* hp;
    hp = gethostbyname(servername);
    if (!hp) {
        perror("Feil for å motta server IP");
        return 0;
    }

    memset(&(peer->addr), 0, sizeof(peer->addr));
    peer->addr.sin_family = AF_INET;
    memcpy(&(peer->addr.sin_addr), hp->h_addr, hp->h_length);
    peer->addr.sin_port = htons(server_port);

    return 1;
} 

int d1_recv_data(D1Peer *client, char *buffer, size_t sz) {
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    uint8_t packet[1024];
    ssize_t received_bytes = recvfrom(client->socket, packet, 1024, 0, (struct sockaddr*)&from, &fromlen);
    //ssize_t received_bytes = recv(client->socket, packet, 1024, 0);
    if (received_bytes < 0) {
        perror("recvfrom feiler");
        return -1;
    }
    D1Header *header = (D1Header*)packet;
    if (ntohl(header->size) != received_bytes ) {
        printf("Error: pakke størrelse eller sjekksum\n");
        return -1;
    }
    //new line for å teste d2
    int recv_seq = ntohs(header->flags) & SEQNO;

    //slutte new line for d2
    memcpy(buffer, packet + sizeof(D1Header), received_bytes - sizeof(D1Header));
    //d1_send_ack(client, recv_seq);
    if(recv_seq){
        d1_send_ack(client, 1);
    }
    else{
        d1_send_ack(client, 0);
    }

    //d1_send_ack(client, client->next_seqno);
    return received_bytes - sizeof(D1Header);
}

 int d1_wait_ack(D1Peer* peer, char* buffer, size_t sz) {
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    char ackPacket[sizeof(D1Header)];
    struct timeval timeout = {1, 0};   

    setsockopt(peer->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    printf("venter for ACK...\n");

    for (int attempts = 0; attempts < 3; attempts++) {
        ssize_t numBytes = recvfrom(peer->socket, ackPacket, sizeof(D1Header), 0,
                                    (struct sockaddr *)&from, &fromlen);
        if (numBytes < 0) {
            printf("Timeout eller error for ACK. sender igjen packet...\n");
            sendto(peer->socket, buffer, sz, 0, (struct sockaddr *)&(peer->addr), sizeof(peer->addr));
        } else {
            D1Header* recvHeader = (D1Header*)ackPacket;
            printf("Motta ACK med Flags: %04x, Sekvens_nummer: %d\n", ntohs(recvHeader->flags), peer->next_seqno);
            if ((ntohs(recvHeader->flags) & FLAG_ACK) && ((ntohs(recvHeader->flags) & ACKNO) == peer->next_seqno)) {
                peer->next_seqno ^= 1;
                return 1;
            }
        }
    }
    printf("Feil for å motta Ack.\n");
    return -1;
}

 int d1_send_data(D1Peer* peer, char* buffer, size_t sz) {
    char packet[1024];
    memset(packet, 0, sizeof(packet)); 
    D1Header* header = (D1Header*) packet;

    header->flags = htons(FLAG_DATA | (peer->next_seqno << 7));
    header->size = htonl(sz + sizeof(D1Header));

    
    *((uint16_t*)(packet + 2)) = 0;

    memcpy(packet + sizeof(D1Header), buffer, sz); 

    uint16_t checksum = calculate_checksum((uint8_t*)packet, sizeof(D1Header) + sz, sz);
    
    header->checksum = htons(checksum);

    ssize_t sent_bytes = sendto(peer->socket, packet, ntohl(header->size), 0,
                                (struct sockaddr*)&(peer->addr), sizeof(peer->addr));
    if (sent_bytes < 0) {
        perror("Feil for å sende data");
        return -1;
    }

    return d1_wait_ack(peer, packet, ntohl(header->size));
}

void d1_send_ack(D1Peer* peer, int seqno) {
    D1Header header;
    header.flags = htons(FLAG_ACK | (seqno << 0));
    header.size = htonl(sizeof(D1Header));
    header.checksum = 0;

    uint16_t *words = (uint16_t *)&header;
    header.checksum ^= words[0] ^ words[1] ^ words[2];

    sendto(peer->socket, &header, sizeof(header), 0, (struct sockaddr *)&(peer->addr), sizeof(peer->addr));
}





 
