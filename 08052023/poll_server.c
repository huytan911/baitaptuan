#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

int StartsWith(const char *a, const char *b)
{
    if (strncmp(a, b, strlen(b)) == 0)
        return 1;
    return 0;
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct pollfd fds[64];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if (nfds == 64)
            {
                // Tu choi ket noi
                close(client);
            }
            else
            {
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;

                printf("New client connected: %d\n", client);
            }
        }

        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    close(fds[i].fd);
                    // Xoa khoi mang
                    if (i < nfds - 1)
                        fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                }
                else
                {
                    buf[ret] = 0;
                    char *mess = "Sai cu phap";
                    while (1)
                    {
                        if(StartsWith(buf, "client_id:"))
                            break;

                        send(fds[i].fd, mess, strlen(mess), 0);
                        ret = recv(fds[i].fd, buf, sizeof(buf), 0);    
                    }
                    mess = "Ket noi thanh cong";
                    send(fds[i].fd, mess, strlen(mess), 0);
                    char client_id[64];
                    int n = strlen(buf);
                    strncpy(client_id, buf + 11, n);
                    strtok(client_id, "\n");
                    while (1)
                    {
                        ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                        buf[ret] = 0;
                        char message[128];
                        strcpy(message, client_id);
                        strcat(message, ": ");
                        strcat(message, buf);
                        for (int j = 1; j < nfds; j++)
                        {
                            if(j != i)
                            {
                                send(fds[j].fd, message, strlen(message), 0);                                 
                            }
                        }
                        memset(message, 0, strlen(message));
                    }
                }
            }
        }
    }
close(listener);

return 0;
}