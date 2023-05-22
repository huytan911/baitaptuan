#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>


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
    addr.sin_port = htons(9060);

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
                char snum_clients[5];
                sprintf(snum_clients, "%d", nfds);
                char hello[128] = "Xin chào hiện đang có ";
                strcat(hello, snum_clients);
                strcat(hello, " clients đang kết nối");
                ret = send(client, hello, strlen(hello), 0);
                printf("New client connected: %d\n", client);
            }
        }

        for (int i = 1; i < nfds; i++){
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
                    while(buf != "exit"){
                        for(int i = 0; i <= strlen(buf); i++){
                            if (buf[i] != ' '){
                                strncpy(buf, buf + i, strlen(buf));
                                break;
                            }
                        }
                        for(int i = strlen(buf); i >= 0; i--){
                            if(buf[i] != ' '){
                                strncpy(buf, buf + 0, i);
                                break;
                        }
                        }
                        ret = send(fds[i].fd, buf, strlen(buf), 0);
                        ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                        buf[ret] = 0;
                        getchar();
                    }
                    char mess[10] = "Tam biet";
                    ret = send(fds[i].fd, mess, strlen(mess), 0);
                    close(fds[i].fd);   
                }
            }
        }
    }
close(listener);

return 0;
}