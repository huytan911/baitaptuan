#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char buf[1024];
    struct sockaddr_in sender_addr;
    int sender_addr_len = sizeof(sender_addr);

    while (1)
    {
        int ret = recvfrom(receiver, buf, sizeof(buf), 0,
            (struct sockaddr *)&sender_addr, &sender_addr_len);
        if (ret < sizeof(buf))
            buf[ret] = 0;
        FILE *f = fopen("udp_file_receiver.txt", "a");
        fprintf(f, "%s", buf);
        fclose(f);
    }
}