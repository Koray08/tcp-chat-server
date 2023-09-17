#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

char msg[500];

int is_nickname_too_long(const char *nickname) {
    return strlen(nickname) > 100;
}

void *recvmg(void *my_sock) {
    int sock = *((int *)my_sock);
    int len;
    while ((len = recv(sock, msg, 500, 0)) > 0) {
        msg[len] = '\0';
        fputs(msg, stdout);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./client <nickname>\n");
        return 1;
    }

    char *client_name = argv[1];

    if (is_nickname_too_long(client_name)) {
        printf("Nickname should be shorter (max 100 characters).\n");
        return 1;
    }

    pthread_t recvt;
    int len;
    int sock;
    struct sockaddr_in ServerIp;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    ServerIp.sin_port = htons(1234);
    ServerIp.sin_family = AF_INET;
    ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ((connect(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp))) == -1) {
        printf("\nConnection to socket failed\n");
        return 1;
    }

    pthread_create(&recvt, NULL, (void *)recvmg, &sock);

    len = write(sock, client_name, strlen(client_name));
    if (len < 0) {
        printf("\nMessage not sent\n");
        return 1;
    }

    while (fgets(msg, 500, stdin) > 0) {
        len = write(sock, msg, strlen(msg));
        if (len < 0) {
            printf("\nMessage not sent\n");
            return 1;
        }
    }

    pthread_join(recvt, NULL);

    char disconnect_msg[500];
    sprintf(disconnect_msg, "%s disconnected", client_name);
    len = write(sock, disconnect_msg, strlen(disconnect_msg));
    if (len < 0) {
        printf("\nMessage not sent\n");
        return 1;
    }

    shutdown(sock, SHUT_RDWR);

    return 0;
}
