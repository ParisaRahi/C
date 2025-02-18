#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "d2_lookup.h"
#include "d1_udp.h"

D2Client* d2_client_create(const char* server_name, uint16_t server_port) {
    D2Client* client = malloc(sizeof(D2Client));
    if (client == NULL) {
        fprintf(stderr, "Feil for å allokere D2Client\n");
        return NULL;
    }

    client->peer = d1_create_client();
    if (client->peer == NULL) {
        free(client);
        return NULL;
    }

    if (!d1_get_peer_info(client->peer, server_name, server_port)) {
        d1_delete(client->peer);
        free(client);
        return NULL;
    }

    return client;
}

D2Client* d2_client_delete(D2Client* client) {
    if (client) {
        d1_delete(client->peer);
        free(client);
    }
    return NULL;
}

int d2_send_request(D2Client* client, uint32_t id) {
    PacketRequest request;
    request.type = htons(TYPE_REQUEST);
    request.id = htonl(id);

    // Sender data og venter for ack
    if (d1_send_data(client->peer, (char*)&request, sizeof(PacketRequest)) < 0) {
        fprintf(stderr, "Feil for å sende request packet\n");
        return -1;
    }

    return 1;
}

int d2_recv_response_size(D2Client* client) {
    PacketResponseSize responseSize;
    int bytesRead = d1_recv_data(client->peer, (char*)&responseSize, sizeof(PacketResponseSize));
    if (bytesRead <= 0) {
        fprintf(stderr, "Feil for å motta response \n");
        return -1;
    }

    if (ntohs(responseSize.type) != TYPE_RESPONSE_SIZE) {
        fprintf(stderr, "uforventet packet type\n");
        return -1;
    }

    return ntohs(responseSize.size);
}

int d2_recv_response(D2Client* client, char* buffer, size_t sz) {
    int bytesRead = d1_recv_data(client->peer, buffer, sz);
    if (bytesRead <= 0) {
        fprintf(stderr, "Feil for å motta response\n");
        return -1;
    }

    return bytesRead;
}

LocalTreeStore* d2_alloc_local_tree(int num_nodes) {
    LocalTreeStore* store = malloc(sizeof(LocalTreeStore) + sizeof(NetNode) * num_nodes + 8);
    if (store == NULL) {
        fprintf(stderr, "Feil for å allokere memmory i d2_alloc_local_tree\n");
        return NULL;
    }

    store->nodes = (NetNode*)(store + 1);
    store->count = num_nodes;
    return store;
}

void d2_free_local_tree(LocalTreeStore* store) {
    if (store) {
        free(store);
    }
}

int d2_add_to_local_tree(LocalTreeStore* store, int node_idx, char* buffer, int buflen) {
    if (store == NULL || buffer == NULL) {
        return -1;
    }

    int offset = 0;
    while (offset < buflen && (node_idx < store->count)) {
        NetNode* node = (NetNode*)(buffer + offset);
        if (sizeof(NetNode) + offset > buflen) {
            fprintf(stderr, "Buffer overflow \n");
            return -1;
        }

        store->nodes[node_idx++] = *node;
        offset += sizeof(NetNode);
        //sjekker error i d2_test_client
        printf("**************OFFSET: ************", offset);
    }

    return node_idx;
}

void d2_print_tree(LocalTreeStore* store) {
    if (store == NULL) {
        fprintf(stderr, "Local tree store is NULL\n");
        return;
    }

    for (int i = 0; i < store->count; i++) {
        NetNode* node = &store->nodes[i];
        printf("id %d value %d children %d\n", node->id, node->value, node->num_children);
        for (uint32_t j = 0; j < node->num_children; j++) {
            printf("-- id %d\n", node->child_id[j]);
        }
    }
}
