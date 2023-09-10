#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

char msg[500];

void *recvmg(void *my_sock) {
    int sock = *((int *)my_sock);
    int len;
    while ((len = recv(sock, msg, 500, 0)) > 0) {
        msg[len] = '\0';
        fputs(msg, stdout);
    }
}

int main(int argc, char *argv[]) {
    pthread_t recvt;
    int len;
    int sock;
    struct sockaddr_in ServerIp;
    char client_name[100];
    strcpy(client_name, argv[1]);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    ServerIp.sin_port = htons(1234);
    ServerIp.sin_family = AF_INET;
    ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");
    if ((connect(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp))) == -1)
        printf("\n connection to socket failed \n");

    pthread_create(&recvt, NULL, (void *)recvmg, &sock);

    len = write(sock, client_name, strlen(client_name));
    if (len < 0) {
        printf("\n message not sent \n");
    }

    while (fgets(msg, 500, stdin) > 0) {
        len = write(sock, msg, strlen(msg));
        if (len < 0) {
            printf("\n message not sent \n");
        }
    }

    pthread_join(recvt, NULL);

    char disconnect_msg[500];
    sprintf(disconnect_msg, "%s disconnected", client_name);
    len = write(sock, disconnect_msg, strlen(disconnect_msg));
    if (len < 0) {
        printf("\n message not sent \n");
    }

    shutdown(sock, SHUT_RDWR);

    return 0;
}
