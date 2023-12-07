#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <string.h>
#define PORTC 4444
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
    char random_char = 'A' + (rand() % 26);
    const char *socket_path = "/tmp/mysocket1";
    if (access(socket_path, F_OK) != -1)
    {
        if (unlink(socket_path) == -1)
        {
            perror("Failed to remove existing socket file");
            exit(1);
        }
    }
    int usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un usockaddr;
    usockaddr.sun_family = AF_UNIX;
    strcpy(usockaddr.sun_path, socket_path); // Replace with your path

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

    printf("Process C is waiting to receive the file descriptor from R...\n");

    int nsfd = accept(usfd, NULL, NULL);
    // printf("I can accept") ;
    if (nsfd == -1)
    {
        perror("Accept failed");
        exit(1);
    }
    int received_fd = receive_fd(nsfd);
    if (received_fd >= 0)
    {
        printf("Received file descriptor from R: %d\n", received_fd);
        printf("Hey , you are connected to cultural server , just enter your character: \n");
        char b;
        char buff[10];
        int bytes_received = read(received_fd, buff, sizeof(buff));
        b = buff[0];
        buff[1] = '\0';
        // printf("Received from P1 to R server: %s\n", buff);
        printf("You gave character: %c\n", b);

        // open a UDP for scoreboard
        int sockfd;
        struct sockaddr_in serverAddr;
        char buffer[BUFFER_SIZE];

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
        serverAddr.sin_port = htons(PORTC);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.4");

        if (b == random_char)
            strcpy(buffer, "User won :) ");
        else
            strcpy(buffer, "User lost (: ");
        sleep(10);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        close(received_fd);
    }
    else
    {
        printf("Failed to receive the file descriptor from R.\n");
    }

    close(usfd);
    return 0;
}
