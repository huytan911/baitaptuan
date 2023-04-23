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
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(atoi(argv[1]));

    char buf[256];
    char filename[100];
    while (1)
    {
        printf("Enter filename: ");
        fgets(filename, 100, stdin);
        char line[100];
        FILE *f = fopen("udp_file_sender.txt", "r");
        fgets(line, 100, f);
        strcpy(buf, line);
        while (fgets(line, 100, f))
        {
            strcat(buf, line);
        }
        printf("%s", buf);
        int ret = sendto(sender, buf, strlen(buf), 0,
                         (struct sockaddr *)&addr, sizeof(addr));
        fclose(f);
    }
}