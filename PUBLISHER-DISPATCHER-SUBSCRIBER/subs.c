#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/udp.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_CLIENTS 3
struct Message
{
    int id;
    char data[1024];
};

void sendIntArray(int socket, int *array, size_t arraySize)
{
    ssize_t bytesSent = send(socket, array, sizeof(array[0]) * arraySize, 0);
    if (bytesSent == -1)
    {
        perror("Error sending array");
    }
    else
    {
        printf("Sent %zd bytes\n", bytesSent);
    }
}

int main()
{
    // Create a UNIX socket
    int clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un serverAddress;
    serverAddress.sun_family = AF_UNIX;
    strcpy(serverAddress.sun_path, "server_socket2");

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("Error connecting to server");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // Receive data from the server
    struct Message receivedMessage[MAX_CLIENTS];
    ssize_t bytesRead = recv(clientSocket, &receivedMessage, sizeof(receivedMessage), 0);
    if (bytesRead > 0)
    {
        // Process the received message
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            printf("The Options for subscribing are \n");
            printf("Publisher %d      Genre %s\n", receivedMessage[i].id, receivedMessage[i].data);
        }
    }

    // Cleanup
    int oopt[MAX_CLIENTS] = {0};

    // Collect subscription choices from the user
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        printf("Do you want to subscribe for Publisher %d? [YES(1) / NO(0)]\n", i + 1);
        int a;
        scanf("%d", &a);
        oopt[i] = a;
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (oopt[i] == 1)
        {
            printf("You have subscribed for Publisher %d\n", i + 1);
        }
    }

    sendIntArray(clientSocket, oopt, MAX_CLIENTS);

    if (oopt[0] == 1)
    {
        int rsfd = socket(AF_INET, SOCK_RAW, 100);
        if (rsfd < 0)
        {
            printf("Error in creating raw socket");
            exit(0);
        }

        char buffer[4026];
        struct sockaddr_in client;
        int len = sizeof(client);
        int n = recvfrom(rsfd, buffer, 100, 0, (struct sockaddr *)&client, (socklen_t *)&len);
        if (n < 0)
        {
            printf("Error in recieving packet");
            exit(0);
        }
        struct iphdr *ip = (struct iphdr *)buffer;
        for (int i = ip->ihl * 4; i < (ip->ihl * 4) + 36; i++)
        {
            printf("%c", buffer[i]);
        }
        printf("\n");
    }

    if (oopt[1] == 1)
    {
        int rsfd = socket(AF_INET, SOCK_RAW, 200);
        if (rsfd < 0)
        {
            printf("Error in creating raw socket");
            exit(0);
        }

        char buffer[4026];
        struct sockaddr_in client;
        int len = sizeof(client);
        int n = recvfrom(rsfd, buffer, 200, 0, (struct sockaddr *)&client, (socklen_t *)&len);
        if (n < 0)
        {
            printf("Error in recieving packet");
            exit(0);
        }
        struct iphdr *ip = (struct iphdr *)buffer;
        for (int i = ip->ihl * 4; i < (ip->ihl * 4) + 36; i++)
        {
            printf("%c", buffer[i]);
        }
        printf("\n");
    }

    if (oopt[2] == 1)
    {
        int rsfd = socket(AF_INET, SOCK_RAW, 180);
        if (rsfd < 0)
        {
            printf("Error in creating raw socket");
            exit(0);
        }

        char buffer[4026];
        struct sockaddr_in client;
        int len = sizeof(client);
        int n = recvfrom(rsfd, buffer, 180, 0, (struct sockaddr *)&client, (socklen_t *)&len);
        if (n < 0)
        {
            printf("Error in recieving packet");
            exit(0);
        }
        struct iphdr *ip = (struct iphdr *)buffer;
        for (int i = ip->ihl * 4; i < (ip->ihl * 4) + 36; i++)
        {
            printf("%c", buffer[i]);
        }
        printf("\n");
    }

    close(clientSocket);

    return 0;
}
