#include <stdio.h>       // Replace cstdio with stdio.h for C standard I/O functions
#include <sys/socket.h>  // Sockets functions
#include <arpa/inet.h>   // Functions for internet operations
#include <stdlib.h>      // Replace cstdlib with stdlib.h for standard library functions
#include <string.h>      // Memory and string functions
#include <unistd.h>      // UNIX standard functions

#define BUFFSIZE 32

void DieWithError(const char *message) {
    perror(message);  // Print the error message to stderr
    exit(1);
}

int main(int argc, char *argv[]) {
    int sock;  // Socket descriptor
    struct sockaddr_in echoServAddr, fromAddr; // Server address and source address
    char buffer[BUFFSIZE];  // Buffer for echo string
    socklen_t fromSize = sizeof(fromAddr);  // From address size
    int totalBytesRcvd;  // Total bytes received
    int messagesReceived = 0;  // Counter for the number of messages received

    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "USAGE: client2 <server_ip> <port>\n");
        exit(1);
    }

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("Failed to create socket");
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));  // Zero out the structure
    echoServAddr.sin_family = AF_INET;  // Internet address family
    echoServAddr.sin_addr.s_addr = inet_addr(argv[1]);  // Server IP address
    echoServAddr.sin_port = htons(atoi(argv[2]));  // Server port

    printf("Waiting for the server to send data...\n");

    while (1) {  // Infinite loop
        if ((totalBytesRcvd = recvfrom(sock, buffer, BUFFSIZE - 1, 0, (struct sockaddr *) &fromAddr, &fromSize)) <= 0) {
            DieWithError("Failed to receive initial bytes from server");
        }

        buffer[totalBytesRcvd] = '\0';  // Null-terminate the received data

        printf("Received and Send to Server3: %s", buffer);  // Echo the buffer

        messagesReceived++;
        if (messagesReceived >= 6) {
            close(sock);  // Close the socket
            break;  // Exit the loop
        }
    }

    return 0;
}

