#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define PORTN 2222
#define BUFFER_SIZE 10

int main()
{
    int csfd;

    struct sockaddr_in serverAddr;

    // Create Socket
    csfd = socket(AF_INET, SOCK_STREAM, 0);
   // printf("here");
    if (csfd == -1)
    {
        printf("not here");
        perror("Socket creation failed");
        exit(1);
    }
   
    // Set socket info
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORTN);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.2");

    // Connect to the server
    
    if (connect(csfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Connection failed");
        exit(1);
    }

    // Send an integer from the client
    char c;
    printf("Enter a character (A-Z): ");
    if (scanf("%c", &c) == 1)
    {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "%c", c);
        send(csfd, buffer, strlen(buffer), 0);
        printf("Sent from P1 client to R: %c\n", c);
    }
    else
    {
        printf("Invalid input. Please enter a valid character.\n");
    }
    sleep(5);
    return 0;
}
