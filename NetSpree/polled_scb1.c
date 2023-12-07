#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/poll.h> // Include the poll header

#define PORTT 7777
#define PORTC 8888
#define BUFFER_SIZE 1024

int main()
{
    int sockfT, sockfC;
    struct sockaddr_in serverAddrT, serverAddrC;
    socklen_t addr_sizeT, addr_sizeC;
    char bufferT[BUFFER_SIZE], bufferC[BUFFER_SIZE];

    // Create UDP socket for technical server
    sockfT = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfT < 0)
    {
        perror("Socket creation for technical server failed");
        exit(1);
    }

    // Set server address structure for technical server
    memset(&serverAddrT, 0, sizeof(serverAddrT));
    serverAddrT.sin_family = AF_INET;
    serverAddrT.sin_port = htons(PORTT);
    serverAddrT.sin_addr.s_addr = inet_addr("127.0.0.3");

    // Bind the socket to the technical server address
    if (bind(sockfT, (struct sockaddr *)&serverAddrT, sizeof(serverAddrT)) < 0)
    {
        perror("Bind for technical server failed");
        exit(1);
    }

    // Create UDP socket for cultural server
    sockfC = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfC < 0)
    {
        perror("Socket creation for cultural server failed");
        exit(1);
    }

    // Set server address structure for cultural server
    memset(&serverAddrC, 0, sizeof(serverAddrC));
    serverAddrC.sin_family = AF_INET;
    serverAddrC.sin_port = htons(PORTC);
    serverAddrC.sin_addr.s_addr = inet_addr("127.0.0.4");

    // Bind the socket to the cultural server address
    if (bind(sockfC, (struct sockaddr *)&serverAddrC, sizeof(serverAddrC)) < 0)
    {
        perror("Bind for cultural server failed");
        exit(1);
    }

    addr_sizeT = addr_sizeC = sizeof(struct sockaddr_in);

    struct pollfd fds[2];
    int timeout = 10000; // Timeout in milliseconds (adjust as needed)

    // Initialize the pollfd structure for sockfT (technical server)
    fds[0].fd = sockfT;
    fds[0].events = POLLIN;

    // Initialize the pollfd structure for sockfC (cultural server)
    fds[1].fd = sockfC;
    fds[1].events = POLLIN;

    while (1)
    {
        int ret = poll(fds, 2, timeout);
        if (ret < 0)
        {
            perror("poll");
            break;
        }

        if (ret == 0)
        {
            // Timeout occurred (no data received)
            continue;
        }

        if (fds[0].revents & POLLIN)
        {
            // Data available from technical server
            ssize_t bytes_receivedT = recvfrom(sockfT, bufferT, BUFFER_SIZE, 0, (struct sockaddr *)&serverAddrT, &addr_sizeT);
            bufferT[bytes_receivedT] = '\0';
            printf("Received from technical server: %s\n", bufferT);
        }

        if (fds[1].revents & POLLIN)
        {
            // Data available from cultural server
            ssize_t bytes_receivedC = recvfrom(sockfC, bufferC, BUFFER_SIZE, 0, (struct sockaddr *)&serverAddrC, &addr_sizeC);
            bufferC[bytes_receivedC] = '\0';
            printf("Received from cultural server: %s\n", bufferC);
        }
    }

    close(sockfT);
    close(sockfC);

    return 0;
}