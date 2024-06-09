#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdint.h> // Include stdint.h for intptr_t and uintptr_t

#define BUFFSIZE 32
#define NUM_FIGHTERS 6
const char* SEMAPHORE_NAME = "/fighter_semaphore";

typedef struct {
    int strength;
    int defeated;
} Fighter;

Fighter fighters[NUM_FIGHTERS];

void DieWithError(const char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

void* fight(void* args) {
    int* data = (int*)args;
    int idx1 = data[0];
    int idx2 = data[1];
    sem_t* sem = (sem_t*)(intptr_t)data[2]; // Corrected cast using intptr_t
    free(args);

    Fighter* fighter1 = &fighters[idx1];
    Fighter* fighter2 = &fighters[idx2];

    sleep(fighter2->strength / (fighter1->strength + 1)); // Simulating computation time

    printf("Бой: Боец с силой %d против бойца с силой %d\n", fighter1->strength, fighter2->strength);
    if (fighter1->strength > fighter2->strength) {
        fighter1->strength += fighter2->strength;
        fighter2->defeated = 1;
        printf("Победитель: Боец с силой %d; Новая сила: %d\n", fighter1->strength - fighter2->strength, fighter1->strength);
    } else {
        fighter2->strength += fighter1->strength;
        fighter1->defeated = 1;
        printf("Победитель: Боец с силой %d; Новая сила: %d\n", fighter2->strength - fighter1->strength, fighter2->strength);
    }

    sem_post(sem);
    return NULL;
}

void sim(Fighter fighters[]) {
    sem_t* semaphore = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 0);
    if (semaphore == SEM_FAILED) {
        fprintf(stderr, "Ошибка создания семафора: %s\n", strerror(errno));
        return;
    }

    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        int* data = malloc(3 * sizeof(int));
        data[0] = 2 * i;
        data[1] = 2 * i + 1;
        data[2] = (intptr_t)semaphore; // Store semaphore as intptr_t
        if (pthread_create(&threads[i], NULL, fight, data) != 0) {
            free(data);
            DieWithError("Failed to create thread");
        }
    }

    for (int i = 0; i < 3; i++) {
        sem_wait(semaphore);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_close(semaphore);
    sem_unlink(SEMAPHORE_NAME);
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoServAddr, fromAddr;
    char buffer[BUFFSIZE];
    socklen_t fromSize = sizeof(fromAddr);
    int totalBytesRcvd;
    int messagesReceived = 0;

    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "USAGE: client3 <server_ip> <port>\n");
        exit(1);
    }

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("Failed to create socket");
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(argv[1]);
    echoServAddr.sin_port = htons(atoi(argv[2]));

    printf("Waiting for the server to send data...\n");

    while (1) {
        if ((totalBytesRcvd = recvfrom(sock, buffer, BUFFSIZE - 1, 0, (struct sockaddr *)&fromAddr, &fromSize)) <= 0) {
            DieWithError("Failed to receive initial bytes from server");
        }

        buffer[totalBytesRcvd] = '\0';

        int energy = atoi(buffer);
        fighters[messagesReceived].strength = energy;
        fighters[messagesReceived].defeated = 0;

        messagesReceived++;
        if (messagesReceived >= 6) {
            close(sock);
            sim(fighters);
            break;
        }
    }

    return 0;
}

