#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>


#define PORT 5555
#define PORTN 6666
#define BUFFER_SIZE 10


int send_fd(int socket, int fd)
{
    struct msghdr msg = {0};
    char buf[CMSG_SPACE(sizeof(int))];
    memset(buf, 0, sizeof(buf));

    struct iovec io = {.iov_base = " ", .iov_len = 1};
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    *((int *)CMSG_DATA(cmsg)) = fd;

    if (sendmsg(socket, &msg, 0) == -1)
    {  
        perror("sendmsg");
        return -1;
    }

    return 0;
}

int main()
{
    int sfd, sfd1;
    struct sockaddr_in add, addr;
    socklen_t addrlen = sizeof(add);
    socklen_t addrlent = sizeof(addr);
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }
    if (sfd == -1)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&add, 0, sizeof(add));
    add.sin_family = AF_INET;
    add.sin_port = htons(PORT);
    add.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sfd, (struct sockaddr *)&add, sizeof(add)) == -1)
    {
        perror("Bind failed");
        exit(1);
    }

    if (listen(sfd, 3) == -1)
    {
        perror("Listen failed");
        exit(1);
    }

    sfd1 = socket(AF_INET, SOCK_STREAM, 0);
    int opt1 = 1;
    if (setsockopt(sfd1, SOL_SOCKET, SO_REUSEADDR, &opt1, sizeof(opt1)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    if (sfd1 == -1)
    {
        perror("Socket creation failed");
        exit(1);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORTN);
    addr.sin_addr.s_addr = inet_addr("127.0.0.2");
    if (bind(sfd1, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Bind failed");
        exit(1);
    }
    if (listen(sfd1, 3) == -1)
    {
        perror("Listen failed");
        exit(1);
    }

    const char *socket_path = "/tmp/mysocket";
    const char *socket_path1 = "/tmp/mysocket1";
    if (access(socket_path1, F_OK) != -1)
    {
        if (unlink(socket_path1) == -1)
        {
            perror("Failed to remove existing socket file");
            exit(1);
        }
    }

    if (access(socket_path, F_OK) != -1)
    {
        if (unlink(socket_path) == -1)
        {
            perror("Failed to remove existing socket file");
            exit(1);
        }
    }

    struct pollfd fds[2];
    int timeout = 5000; 
    fds[0].fd = sfd;
    fds[0].events = POLLIN;
    fds[1].fd = sfd1;
    fds[1].events = POLLIN;

    while (1)
    {
        if (poll(fds, 2, timeout) > 0)
        {
            if (fds[0].revents & POLLIN)
            {
                struct sockaddr_in cli;
                socklen_t len = sizeof(cli);
                int nsfd = accept(sfd, (struct sockaddr *)&cli, &len);
                int c = fork();

                if (c == 0)
                {
                    
                    int usfd = socket(AF_UNIX, SOCK_STREAM, 0);
                    struct sockaddr_un usockaddr;
                    usockaddr.sun_family = AF_UNIX;
                    strcpy(usockaddr.sun_path, socket_path);
                    sleep(10);
                    if (connect(usfd, (struct sockaddr *)&usockaddr, sizeof(usockaddr)) == -1)
                    {
                        perror("Unix socket connection failed technical");
                        exit(1);
                    }
                    if (send_fd(usfd, nsfd) == -1)
                    {
                        perror("send_fd");
                        exit(1);
                    }

                    close(usfd);
                }
            }

            if (fds[1].revents & POLLIN)
            {
                
                struct sockaddr_in clie;
                socklen_t lent = sizeof(clie);
                int nsfd1 = accept(sfd1, (struct sockaddr *)&clie, &lent);
                int c1 = fork();

                if (c1 == 0)
                {
                    
                    int usfd1 = socket(AF_UNIX, SOCK_STREAM, 0);
                    struct sockaddr_un usockaddr1;
                    usockaddr1.sun_family = AF_UNIX;
                    strcpy(usockaddr1.sun_path, socket_path1);
                    sleep(10);
                    if (connect(usfd1, (struct sockaddr *)&usockaddr1, sizeof(usockaddr1)) == -1)
                    {
                        perror("Unix socket connection failed cultural");
                        exit(1);
                    }
                    if (send_fd(usfd1, nsfd1) == -1)
                    {
                        perror("send_fd");
                        exit(1);
                    }
                    close(usfd1);
                }
            }
        }
        else if (poll < 0)
        {
            perror("poll");
        }
    }

    return 0;
}
