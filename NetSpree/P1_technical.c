#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 1111
#define BUFFER_SIZE 10

int main()
{
    int csfd;

    struct sockaddr_in serverAddr;

    // Create Socket
    csfd = socket(AF_INET, SOCK_STREAM, 0);

    if (csfd == -1)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Set socket info
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(csfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Connection failed");
        exit(1);
    }

    // Send an integer from the client
    int num;
    printf("Enter an integer (1-50): ");
    if (scanf("%d", &num) == 1)
    {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "%d", num);
        send(csfd, buffer, strlen(buffer), 0);
        printf("Sent from P1 client to R: %s\n", buffer);
    }
    else
    {
        printf("Invalid input. Please enter a valid integer.\n");
    }
    sleep(5);
 
    return 0;
}
