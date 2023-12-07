#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
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
#define BUFFER_SIZE 1024

struct Message
{
    int id;
    char msg[BUFFER_SIZE];
};

struct Message Regis[MAX_CLIENTS];

struct Subss
{
    int Sno;
    int opted[MAX_CLIENTS];
};

struct Subss S[MAX_CLIENTS];

void *handle_client1(void *arg)
{
    int clientSocket = *((int *)arg);
    free(arg);

    // Prepare a message to send
    struct Message message[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        message[i] = Regis[i];
    }

    ssize_t bytesSent = send(clientSocket, &message, sizeof(message), 0);
    if (bytesSent == -1)
    {
        perror("Error sending message");
    }
    int arr[MAX_CLIENTS];
    int bytes_received = recv(clientSocket, arr, sizeof(arr), 0);
    printf("Received from Subscriber %d \n", clientSocket - 3);
    S[clientSocket - 4].Sno = clientSocket - 3;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {

        S[clientSocket - 4].opted[i] = arr[i];
    }

    close(clientSocket);
    return NULL;
}

void *handle_client(void *arg)
{
    int client_socket = *((int *)arg);
    free(arg);

    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Received from Publisher %d: %s", client_socket - 3, buffer);
        Regis[client_socket - 4].id = client_socket - 3;
        strncpy(Regis[client_socket - 4].msg, buffer, BUFFER_SIZE);
    }

    printf("Publisher %d disconnected.\n", client_socket - 3);
    close(client_socket);

    return NULL;
}

int main()
{
    // PUBLISHER PART

    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
    {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(12345);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error binding TCP socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 12345...\n");

    int client_id = 1;
    pthread_t client_threads[MAX_CLIENTS];

    while (client_id <= MAX_CLIENTS)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1)
        {
            perror("Error accepting connection");
            continue;
        }

        int *client_socket_ptr = (int *)malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        if (pthread_create(&client_threads[client_id - 1], NULL, handle_client, client_socket_ptr) != 0)
        {
            perror("Error creating thread");
            free(client_socket_ptr);
            continue;
        }

        printf("Client %d connected.\n", client_id);
        client_id++;
    }

    // Wait for all threads to finish
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        pthread_join(client_threads[i], NULL);
    }

    close(server_socket);
    printf("All publishers details \n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        printf("Registered P%d  with ID: %d      Genre: %s\n", i + 1, Regis[i].id, Regis[i].msg);
    }

    //----------------------------------------------------------------------------------//

    // SUBSCRIBER CONNECTS

    int serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        perror("Error creating server socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un serverAddress;
    serverAddress.sun_family = AF_UNIX;
    strcpy(serverAddress.sun_path, "server_socket2");
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
    {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("Error binding UNIX socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) == -1)
    {
        perror("Error listening for connections");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on UNIX socket: server_socket\n");

    int c1 = MAX_CLIENTS;
    pthread_t tid[MAX_CLIENTS];
    int c_id = 1;
    while (c_id <= MAX_CLIENTS)
    {

        int *clientSocket = malloc(sizeof(int));
        *clientSocket = accept(serverSocket, NULL, NULL);
        if (*clientSocket == -1)
        {
            perror("Error accepting connection");
            free(clientSocket);
            continue;
        }

        if (pthread_create(&tid[c_id - 1], NULL, handle_client1, clientSocket) != 0)
        {
            perror("Error creating thread");
            close(*clientSocket);
            free(clientSocket);
        }
        c_id++;
    }
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        pthread_join(tid[i], NULL);
    }

    printf("\nAll subscriber details \n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        printf("Subscriber S%d opted for ", S[i].Sno);
        for (int k = 0; k < MAX_CLIENTS; k++)
        {
            printf("%d ", S[i].opted[k]);
        }
        printf("\n");
    }

    close(serverSocket);

    // RAW 1
    int raw_socket = socket(AF_INET, SOCK_RAW, 100);

    if (raw_socket == -1)
    {
        perror("Socket creation error");
        exit(1);
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.2");

    char packet[100] = "Hey subscriber , You are watching P1!";

    // Send the message using sendto
    if (sendto(raw_socket, packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == -1)
    {
        perror("Message send error");
    }
    else
    {
        printf("P1 Displayed Successfully\n");
    }

    close(raw_socket);
    sleep(2);
    // RAW 2
    int raw_socket1 = socket(AF_INET, SOCK_RAW, 200);

    if (raw_socket1 == -1)
    {
        perror("Socket creation error");
        exit(1);
    }

    struct sockaddr_in dest_addr1;
    dest_addr1.sin_family = AF_INET;
    dest_addr1.sin_addr.s_addr = inet_addr("127.0.0.3");

    char packet1[100] = "Hey subscriber , You are watching P2!";

    // Send the packet using sendto
    if (sendto(raw_socket1, packet1, sizeof(packet1), 0, (struct sockaddr *)&dest_addr1, sizeof(dest_addr1)) == -1)
    {
        perror("Packet send error");
    }
    else
    {
        printf("P2 Displayed Successfully\n");
    }

    close(raw_socket1);
    sleep(2);
    // RAW 3
    int raw_socket2 = socket(AF_INET, SOCK_RAW, 180);

    if (raw_socket2 == -1)
    {
        perror("Socket creation error");
        exit(1);
    }

    struct sockaddr_in dest_addr2;
    dest_addr2.sin_family = AF_INET;
    dest_addr2.sin_addr.s_addr = inet_addr("127.0.0.4");
    char packet2[100] = "Hey subscriber , You are watching P3!";
    if (sendto(raw_socket2, packet2, sizeof(packet2), 0, (struct sockaddr *)&dest_addr2, sizeof(dest_addr2)) == -1)
    {
        perror("Packet send error");
    }
    else
    {
        printf("P3 Displayed Successfully\n");
    }

    close(raw_socket2);

    return 0;
}
