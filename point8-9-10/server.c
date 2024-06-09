#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFSIZE 32

void DieWithError(const char *mess) {
    perror(mess);
    exit(1);
}

int main(int argc, char *argv[]) {
    int serversock;
    struct sockaddr_in echoServAddr, echoClient;
    char buffer[BUFFSIZE];
    unsigned int recvMsgSize;
    socklen_t clientLen;
    int totalBytesRcvd;

    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    if ((serversock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("Failed to create socket");
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(argv[1]);
    echoServAddr.sin_port = htons(atoi(argv[2]));

    if (bind(serversock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
        DieWithError("Failed to bind the server socket");
    }

    printf("Server started. Waiting for messages...\n");

    struct sockaddr_in *clients = NULL;
    int numClients = 0;
    clientLen = sizeof(echoClient);

    while (1) {
        if ((totalBytesRcvd = recvfrom(serversock, buffer, BUFFSIZE, 0, (struct sockaddr *) &echoClient, &clientLen)) < 0) {
            DieWithError("Failed to receive bytes from client");
        }

        buffer[totalBytesRcvd] = '\0'; // Null-terminate received data
        printf("Received: %s\n", buffer);

        // Check if this client is already in the list
        int knownClient = 0;
        for (int i = 0; i < numClients; i++) {
            if (clients[i].sin_addr.s_addr == echoClient.sin_addr.s_addr &&
                clients[i].sin_port == echoClient.sin_port) {
                knownClient = 1;
                break;
            }
        }

        // Add new client to the list
        if (!knownClient) {
            clients = realloc(clients, (numClients + 1) * sizeof(struct sockaddr_in));
            clients[numClients++] = echoClient;
        }

        // Send the message to all known clients
        for (int i = 0; i < numClients; i++) {
            if (sendto(serversock, buffer, totalBytesRcvd, 0, (struct sockaddr *)&clients[i], clientLen) != totalBytesRcvd) {
                DieWithError("Failed to send bytes to client");
            }
        }
    }

    free(clients);
    close(serversock);

    return 0;
}

