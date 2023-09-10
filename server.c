#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

pthread_mutex_t mutex;
int clients[20]; 
char client_names[20][100];
int n = 0;

void sendtoall(char *msg, int curr) {
    int i;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < n; i++) {
        if (clients[i] != curr) {
            if (send(clients[i], msg, strlen(msg), 0) < 0) {
                printf("sending failure \n");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void notify_client_join(int client_sock, char *client_name) {

    char welcome_msg[500];
    sprintf(welcome_msg, "Welcome to the chat app!\n");
    if (send(client_sock, welcome_msg, strlen(welcome_msg), 0) < 0) {
        printf("sending failure \n");
    }

    char join_msg[500];
    sprintf(join_msg, "%s joined the chat\n", client_name);
    sendtoall(join_msg, -1); 
}

void notify_client_disconnect(char *client_name) {
    char disconnect_msg[500];
    sprintf(disconnect_msg, "%s disconnected from the chat\n", client_name);

    // Remove the disconnected client from the list
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < n; i++) {
        if (strcmp(client_names[i], client_name) == 0) {
            for (int j = i; j < n - 1; j++) {
                clients[j] = clients[j + 1];
                strcpy(client_names[j], client_names[j + 1]);
            }
            n--;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    sendtoall(disconnect_msg, -1); 
}

void *recvmg(void *client_sock) {
    int sock = *((int *)client_sock);
    char msg[500];
    int len;
    char client_name[100];

    len = recv(sock, client_name, sizeof(client_name), 0);
    if (len > 0) {
        client_name[len] = '\0';
        notify_client_join(sock, client_name);
        pthread_mutex_lock(&mutex);
        clients[n] = sock;
        strcpy(client_names[n], client_name);
        n++;
        pthread_mutex_unlock(&mutex);
    }

    while ((len = recv(sock, msg, 500, 0)) > 0) {
        msg[len] = '\0';
        char message_with_name[600];
        sprintf(message_with_name, "%s: %s", client_name, msg);
        sendtoall(message_with_name, sock);
    }

    notify_client_disconnect(client_name);
    shutdown(sock, SHUT_RDWR);

    return NULL;
}

int main() {
    struct sockaddr_in ServerIp;
    int sock = 0, Client_sock = 0;

    ServerIp.sin_family = AF_INET;
    ServerIp.sin_port = htons(1234);
    ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp)) == -1) {
        printf("cannot bind, error!! \n");
    } else {
        printf("Server Started\n");
    }

    if (listen(sock, 20) == -1) {
        printf("listening failed \n");
    }

    while (1) {
        if ((Client_sock = accept(sock, (struct sockaddr *)NULL, NULL)) < 0)
            printf("accept failed  \n");

        pthread_t recvt;
        pthread_create(&recvt, NULL, (void *)recvmg, &Client_sock);
    }

    return 0;
}
