#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFSIZE 32

// Error handling function
void DieWithError(const char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoServAddr;
    char buffer[BUFFSIZE];
    unsigned int recvMsgSize;

    // Check correct usage
    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    // Create a UDP socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("Failed to create socket");
    }

    // Construct the server address structure
    memset(&echoServAddr, 0, sizeof(echoServAddr)); // Zero out structure
    echoServAddr.sin_family = AF_INET;              // Internet address family
    echoServAddr.sin_addr.s_addr = inet_addr(argv[1]); // Server IP address
    echoServAddr.sin_port = htons(atoi(argv[2]));      // Server port

    // Send data to the server
    int n = 7;
    for (int i = 0; i < n; i++) {
        if (i == 0)
            printf("Enter number of future clients: ");
        else
            printf("Enter Ci-energy of fighter %d: ", i);

        fgets(buffer, BUFFSIZE, stdin);
        recvMsgSize = strlen(buffer);

        // Send the string to the server
        if (sendto(sock, buffer, recvMsgSize, 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != recvMsgSize) {
            DieWithError("Mismatch in number of sent bytes");
        }
    }

    // Close the socket
    close(sock);
    return 0;
}

