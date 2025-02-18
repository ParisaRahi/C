/*
 * This is the main program for the proxy, which receives connections for sending and receiving clients
 * both in binary and XML format. Many clients can be connected at the same time. The proxy implements
 * an event loop.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "xmlfile.h"
#include "connection.h"
#include "record.h"
#include "recordToFormat.h"
#include "recordFromFormat.h"

#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>

#define MAX_CLIENTS 100

#define MAX_BUFFER_SIZE 100000

#define MODE_UNSET 0
#define MODE_RECEIVER_XML 1
#define MODE_RECEIVER_BINARY 2
#define MODE_SENDER_XML 3
#define MODE_SENDER_BINARY 4

/* This struct should contain the information that you want
 * keep for one connected client.
 */
struct Client
{
    int sock;                     // The socket descriptor for the client connection
    char buffer[MAX_BUFFER_SIZE]; // The buffer to store incoming data
    int buffer_length;            // The current length of the data in the buffer
    int mode;                     // The mode of the client
    char destination;             // The destination ID for this client
};

typedef struct Client Client;

Client *clients[MAX_CLIENTS];

void usage(char *cmd)
{
    fprintf(stderr, "Usage: %s <port>\n"
                    "       This is the proxy server. It takes as imput the port where it accepts connections\n"
                    "       from \"xmlSender\", \"binSender\" and \"anyReceiver\" applications.\n"
                    "       <port> - a 16-bit integer in host byte order identifying the proxy server's port\n"
                    "\n",
            cmd);
    exit(-1);
}

/*
 * This function is called when a new connection is noticed on the server
 * socket.
 * The proxy accepts a new connection and creates the relevant data structures.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void handleNewClient(int server_sock)
{
    int new_sock = tcp_accept(server_sock);
    if (new_sock < 0)
    {
        perror("Accept failed");
        return;
    }

    Client *client = malloc(sizeof(Client));
    if (client == NULL)
    {
        perror("malloc failed");
        return;
    }

    client->sock = new_sock;
    client->buffer_length = 0;
    client->mode = MODE_UNSET;
    client->destination = 0; // or some default value

    // Add code to read initial bytes to determine client's mode
    char initial_byte;
    int bytes_read = tcp_read(client->sock, &initial_byte, 1);
    if (bytes_read < 0)
    {
        perror("Read initial byte failed");
        free(client);
        return;
    }

    //fprintf(stdout, "initial_byte is : %c", initial_byte);

    switch (initial_byte)
    {
    case '1': // or whatever the XML sender sends
        client->mode = MODE_RECEIVER_XML;
        break;
    case '2': // or whatever the binary sender sends
        client->mode = MODE_RECEIVER_BINARY;
        break;
    case 'X': // or whatever the binary sender sends
        client->mode = MODE_SENDER_XML;
        break;
    case 'B': // or whatever the binary sender sends
        client->mode = MODE_SENDER_BINARY;
        break;
    default:
        fprintf(stderr, "Unknown mode byte %c\n", initial_byte);
        free(client);
        return;
    }

    // Add code to read  bytes to determine client's id
    char id;
    bytes_read = tcp_read(client->sock, &id, 1);
    if (bytes_read < 0)
    {
        perror("Read initial byte failed");
        free(client);
        return;
    }

    client->destination = id;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == NULL)
        {
            clients[i] = client;
            break;
        }
    }
}

/*
 * This function is called when a connection is broken by one of the connecting
 * clients. Data structures are clean up and resources that are no longer needed
 * are released.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void removeClient(Client *client)
{
    // Close the client's socket
    close(client->sock);

    // Remove the client from the clients array
    for (int i = 0; i < FD_SETSIZE; ++i)
    {
        if (clients[i] == client)
        {
            clients[i] = NULL;
            break;
        }
    }

    // Free the memory allocated for the Client structure
    free(client);
}

/*
 * This function is called when the proxy received enough data from a sending
 * client to create a Record. The 'dest' field of the Record determines the
 * client to which the proxy should send this Record.
 *
 * If no such client is connected to the proxy, the Record is discarded without
 * error. Resources are released as appropriate.
 *
 * If such a client is connected, this functions find the correct socket for
 * sending to that client, and determines if the Record must be converted to
 * XML format or to binary format for sendig to that client.
 *
 * It does then send the converted messages.
 * Finally, this function deletes the Record before returning.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void forwardMessage(Record *msg)
{
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        Client *client = clients[i];

        if (client == NULL || client->destination != msg->dest)
        {
            continue;
        }

        char *data;
        int bufSize;
        int bytes_sent;

        if (client->mode == MODE_RECEIVER_XML)
        {
            // Convert the record to XML format
            data = recordToXML(msg, &bufSize);
            if (data == NULL)
            {
                fprintf(stderr, "Error converting record to XML format\n");
                return;
            }

            // Send the XML data to the client
            bytes_sent = tcp_write_loop(client->sock, data, strlen(data));
        }
        else if (client->mode == MODE_RECEIVER_BINARY)
        {
            // Convert the record to binary format
            data = recordToBinary(msg, &bufSize);
            if (data == NULL)
            {
                fprintf(stderr, "Error converting record to Binary format\n");
                return;
            }

            // Send the binary data to the client
            bytes_sent = tcp_write_loop(client->sock, data, bufSize);
        }
        else
        {
            continue;
        }

        if (bytes_sent < 0)
        {
            fprintf(stderr, "Error sending data to client\n");
        }

        free(data);
        break;
    }

    // Clean up the record
    deleteRecord(msg);
}

char **splitBufferIntoXMLRecords(char *buffer, int buffer_size, int *record_count)
{

    const char *start_tag = "<record>";
    const char *end_tag = "</record>";

    char **records = NULL;

    char *current = buffer;

    while (current < buffer + buffer_size)
    {
        char *start = strstr(current, start_tag);
        char *end = strstr(current, end_tag);

        if (start != NULL && end != NULL)
        {
            int record_size = end - start;
            char *record = malloc(record_size + strlen(end_tag) + 1);
            if (record == NULL)
            {
                fprintf(stderr, "Failed to allocate memory for record.\n");
                return NULL;
            }
            memcpy(record, start, record_size + strlen(end_tag));
            record[record_size + strlen(end_tag)] = '\0';

            char **temp = realloc(records, (*record_count + 1) * sizeof(char *));
            if (temp == NULL)
            {
                fprintf(stderr, "Failed to allocate memory for records.\n");

                // Free the currently allocated record
                free(record);

                // Free all previously allocated records
                for (int i = 0; i < *record_count; i++)
                {
                    free(records[i]);
                }

                // Free the records array itself
                free(records);

                return NULL;
            }
            records = temp;
            records[*record_count] = record;
            (*record_count)++;
            current = end + strlen(end_tag);
        }
        else
        {
            break;
        }
    }
    return records;
}

typedef struct
{
    char *data;
    int size;
} RecordBuffer;

RecordBuffer *splitBufferIntoBinaryRecords(char *buffer, int buffer_size, int *record_count)
{
    *record_count = 0;

    RecordBuffer *records = NULL;
    char *current = buffer;

    while (current < buffer + buffer_size)
    {
        // Get the length of the username
        int username_length = ntohl(*((int *)(current + 3)));

        printf("\n username_length is %d \n", username_length);

        // Calculate the total record size
        int record_size = 1 + 1 + 1 + 4 + username_length + 4 + 4 + 1 + 1 + 2;

        // Check if the record fits in the buffer
        if (current + record_size > buffer + buffer_size)
        {
            break;
        }

        // Allocate a new record buffer
        char *record = malloc(record_size);
        memcpy(record, current, record_size);
        records = realloc(records, (*record_count + 1) * sizeof(RecordBuffer));
        records[*record_count].data = record;
        records[*record_count].size = record_size;
        (*record_count)++;

        current += record_size;
    }

    return records;
}

/*
 * This function is called whenever activity is noticed on a connected socket,
 * and that socket is associated with a client. This can be sending client
 * or a receiving client.
 *
 * The calling function finds the Client structure for the socket where acticity
 * has occurred and calls this function.
 *
 * If this function receives data that completes a record, it creates an internal
 * Record data structure on the heap and calls forwardMessage() with this Record.
 *
 * If this function notices that a client has disconnected, it calls removeClient()
 * to release the resources associated with it.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void handleClient(Client *client)
{

    if (client == NULL)
    {
        // The client has disconnected
        removeClient(client);
        return;
    }

    // Buffer to store the received data
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int bytes_read = tcp_read(client->sock, buffer, MAX_BUFFER_SIZE - client->buffer_length - 1);
    if (bytes_read > 0)
    {
        // Process the received data based on the client mode
        if (client->mode == MODE_SENDER_XML)
        {

            int record_count = 0;
            char **records = splitBufferIntoXMLRecords(buffer, bytes_read, &record_count);

            for (int r = 0; r < record_count; r++)
            {

                // Make sure the client has been allocated correctly
                if (client == NULL)
                {
                    fprintf(stderr, "Client is NULL\n");
                    // The client has disconnected
                    removeClient(client);
                    return;
                }

                // Make sure buffer_length + strlen(records[r]) doesn't exceed MAX_BUFFER_SIZE
                if (client->buffer_length + strlen(records[r]) >= MAX_BUFFER_SIZE)
                {
                    fprintf(stderr, "Buffer overflow: buffer_length + strlen(records[r]) exceeds MAX_BUFFER_SIZE\n");
                    // The client has disconnected
                    removeClient(client);
                    return;
                }

                // Append the received data to the client's buffer
                int total_length = client->buffer_length + strlen(records[r]);

                if (total_length < MAX_BUFFER_SIZE - client->buffer_length - 1)
                {

                    for (int i = 0; i < (int)strlen(records[r]); i++)
                    {
                        // Check that i is within range
                        if (i >= MAX_BUFFER_SIZE)
                        {
                            fprintf(stderr, "Index out of range: i is greater than MAX_BUFFER_SIZE\n");
                            // The client has disconnected
                            removeClient(client);
                            return;
                        }

                        // Check that buffer[i] is within range
                        if (records[r][i] == '\0')
                        {
                            fprintf(stderr, "Unexpected null character in buffer at index %d\n", i);
                            // The client has disconnected
                            removeClient(client);
                            return;
                        }
                        client->buffer[client->buffer_length + i] = records[r][i];
                    }
                    client->buffer_length += strlen(records[r]);
                    client->buffer[client->buffer_length] = '\0';
                }
                else
                {
                    fprintf(stderr, "Client buffer overflow\n");
                    // The client has disconnected
                    removeClient(client);
                    return;
                }

                // Check if a complete record has been received
                int *bytesread = malloc(sizeof(int));

                Record *record = XMLtoRecord(records[r], strlen(records[r]), bytesread);
                if (record != NULL)
                {
                    // Forward the message and clean up the record
                    forwardMessage(record);
                }
                free(bytesread);
            }
            // free each individual record
            for (int i = 0; i < record_count; i++)
            {
                free(records[i]);
            }

            // free the records array itself
            free(records);
        }
        // else if (client->mode == MODE_SENDER_BINARY)
        // {
        //     int record_count;

        //     RecordBuffer *records = splitBufferIntoBinaryRecords(buffer, bytes_read, &record_count);

        //     printf("\n record_count is %d \n", record_count);

        //     for (int r = 0; r < record_count; r++)
        //     {

        //         // Make sure the client has been allocated correctly
        //         if (client == NULL)
        //         {
        //             fprintf(stderr, "Client is NULL\n");
        //             // The client has disconnected
        //             removeClient(client);
        //             return;
        //         }

        //         // Make sure buffer_length + records[r].size doesn't exceed MAX_BUFFER_SIZE
        //         if (client->buffer_length + records[r].size >= MAX_BUFFER_SIZE)
        //         {
        //             fprintf(stderr, "Buffer overflow: buffer_length + bytes_read exceeds MAX_BUFFER_SIZE\n");
        //             // The client has disconnected
        //             removeClient(client);
        //             return;
        //         }

        //         // Append the received data to the client's buffer
        //         if (client->buffer_length + records[r].size < MAX_BUFFER_SIZE - client->buffer_length - 1)
        //         {
        //             // Copy data from buffer to client's buffer
        //             for (int i = 0; i < records[r].size; i++)
        //             {
        //                 if (i >= MAX_BUFFER_SIZE)
        //                 {
        //                     fprintf(stderr, "Index out of range: i is greater than MAX_BUFFER_SIZE\n");
        //                     // The client has disconnected
        //                     removeClient(client);
        //                     return;
        //                 }

        //                 client->buffer[client->buffer_length + i] = records[r].data[i];
        //             }

        //             client->buffer_length += records[r].size;
        //             // client->buffer[client->buffer_length] = '\0';
        //         }
        //         else
        //         {
        //             fprintf(stderr, "Client buffer overflow\n");
        //             // The client has disconnected
        //             removeClient(client);
        //             return;
        //         }

        //         // Check if a complete record has been received
        //         int *bytesread = malloc(sizeof(int));
        //         Record *record = BinaryToRecord(records[r].data, records[r].size, bytesread);
        //         if (record != NULL)
        //         {
        //             // Forward the message and clean up the record
        //             forwardMessage(record);
        //         }
        //         free(bytesread);
        //         free(records[r].data);
        //     }
        //     free(records);
        // }
        else if (client->mode == MODE_SENDER_BINARY)
        {
            // Make sure the client has been allocated correctly
            if (client == NULL)
            {
                fprintf(stderr, "Client is NULL\n");
                // The client has disconnected
                removeClient(client);
                return;
            }

            // Make sure buffer_length + bytes_read doesn't exceed MAX_BUFFER_SIZE
            if (client->buffer_length + bytes_read >= MAX_BUFFER_SIZE)
            {
                fprintf(stderr, "Buffer overflow: buffer_length + bytes_read exceeds MAX_BUFFER_SIZE\n");
                // The client has disconnected
                removeClient(client);
                return;
            }

            // Append the received data to the client's buffer
            if (client->buffer_length + bytes_read < MAX_BUFFER_SIZE - client->buffer_length - 1)
            {
                // Copy data from buffer to client's buffer
                for (int i = 0; i < bytes_read; i++)
                {
                    if (i >= MAX_BUFFER_SIZE)
                    {
                        fprintf(stderr, "Index out of range: i is greater than MAX_BUFFER_SIZE\n");
                        // The client has disconnected
                        removeClient(client);
                        return;
                    }

                    client->buffer[client->buffer_length + i] = buffer[i];
                }

                client->buffer_length += bytes_read;
                // client->buffer[client->buffer_length] = '\0';
            }
            else
            {
                fprintf(stderr, "Client buffer overflow\n");
                // The client has disconnected
                removeClient(client);
                return;
            }

            // Check if a complete record has been received
            int *bytesread = malloc(sizeof(int));
            Record *record = BinaryToRecord(buffer, bytes_read, bytesread);
            if (record != NULL)
            {
                // Forward the message and clean up the record
                forwardMessage(record);
            }
            free(bytesread);
        }
        else
        {
            fprintf(stderr, "\n Unknown mode %d \n", client->mode);
        }
    }
    else if (bytes_read == 0)
    {
        // The client has disconnected
        removeClient(client);
    }
    else
    {
        // An error occurred while reading from the client
        fprintf(stderr, "Error reading from client\n");
    }
}

int main(int argc, char *argv[])
{
    int port;
    int server_sock;

    if (argc != 2)
    {
        usage(argv[0]);
    }

    port = atoi(argv[1]);

    server_sock = tcp_create_and_listen(port);

    if (server_sock < 0)
    {
        exit(-1);
    }

    fd_set set;

    do
    {
        FD_ZERO(&set);
        FD_SET(server_sock, &set);

        int max_fd = server_sock;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i])
            {
                FD_SET(clients[i]->sock, &set);
                if (clients[i]->sock > max_fd)
                {
                    max_fd = clients[i]->sock;
                }
            }
        }

        if (select(max_fd + 1, &set, NULL, NULL, NULL) < 0)
        {
            perror("Select failed");
            break;
        } 

        if (FD_ISSET(server_sock, &set))
        {
            handleNewClient(server_sock);
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] && FD_ISSET(clients[i]->sock, &set))
            {
                handleClient(clients[i]);
            }
        }
    } while (1); // TODO: Add termination condition.

    tcp_close(server_sock);

    return 0;
}
