#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <string.h>

#define PORTT 3333
#define BUFFER_SIZE 1024

int receive_fd(int socket)
{
    int fd;
    struct msghdr msg = {0};
    char buf[1];
    struct iovec io = {.iov_base = buf, .iov_len = sizeof(buf)};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    struct cmsghdr *cmsg;
    char cms[CMSG_SPACE(sizeof(int))];
    msg.msg_control = cms;
    msg.msg_controllen = sizeof(cms);

    if (recvmsg(socket, &msg, 0) < 0)
    {
        perror("recvmsg");
        return -1;
    }

    cmsg = CMSG_FIRSTHDR(&msg);

    if (cmsg == NULL)
    {
        fprintf(stderr, "No file descriptor received\n");
        return -1;
    }

    if (cmsg->cmsg_level != SOL_SOCKET)
    {
        fprintf(stderr, "Unknown protocol level: %d\n", cmsg->cmsg_level);
        return -1;
    }

    if (cmsg->cmsg_type != SCM_RIGHTS)
    {
        fprintf(stderr, "Unknown control message type: %d\n", cmsg->cmsg_type);
        return -1;
    }

    fd = *((int *)CMSG_DATA(cmsg));
    return fd;
}

int main()
{

    srand(time(NULL));

    // Generate a random number between 0 and 50
    int random_number = rand() % 51;
    // Create a Unix domain socket to receive the file descriptor from R
    const char *socket_path = "/tmp/mysocket";
    int usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un usockaddr;
    usockaddr.sun_family = AF_UNIX;
    strcpy(usockaddr.sun_path, socket_path);

    if (bind(usfd, (struct sockaddr *)&usockaddr, sizeof(usockaddr)) == -1)
    {
        perror("Unix socket binding failed");
        exit(1);
    }

    if (listen(usfd, 1) == -1)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("Process T is waiting to receive the file descriptor from R...\n");

    int nsfd = accept(usfd, NULL, NULL);
    if (nsfd == -1)
    {
        perror("Accept failed");
        exit(1);
    }

    int received_fd = receive_fd(nsfd);

    if (received_fd >= 0)
    {
        printf("Received file descriptor from R: %d\n", received_fd);
        int digit;

        printf("Hey , you are connected to technical server , just getting your number \n");

        // Now, you can use 'received_fd' to communicate with P1 or perform other operations.
        // For example, you can read/write to 'received_fd'.
        char buff[BUFFER_SIZE];
        int bytes_received = read(received_fd, buff, sizeof(buff));
        buff[bytes_received] = '\0';
        // printf("Received from P1 to R server: %s\n", buff);
        digit = atoi(buff);
        printf("You gave integer: %d\n", digit);

        int sockfd;
        struct sockaddr_in serverAddr;
        char buffer1[BUFFER_SIZE];

        // Create UDP socket
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            perror("Socket creation failed");
            exit(1);
        }

        // Set server address structure
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORTT);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.3");
        printf("I have results\n") ;
        if (digit == random_number)
            strcpy(buffer1, "User won :) ");
        else
            strcpy(buffer1, "User lost (: ");

        sleep(10);
        sendto(sockfd, buffer1, strlen(buffer1), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        close(received_fd);
    }
    else
    {
        printf("Failed to receive the file descriptor from R.\n");
    }

    close(usfd);
    return 0;
}
