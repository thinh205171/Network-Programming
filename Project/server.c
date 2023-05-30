#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/select.h>

#include "account.h"
#include "check_login.h"

#define BACKLOG 20
#define BUFF_SIZE 1024
#define MAX_CLIENTS 10

extern Account accounts[MAX_ACCOUNTS];
extern int num_accounts;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    int PORT = atoi(argv[1]);
    load_accounts();

    int listen_sock, conn_sock;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t sin_size;

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket() error\n");
        return 0;
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    if (listen(listen_sock, BACKLOG) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    int client_sockets[MAX_CLIENTS], max_sd, sd, activity;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
    }

    fd_set readfds;

    while (1)
    {
        FD_ZERO(&readfds);

        FD_SET(listen_sock, &readfds);
        max_sd = listen_sock;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];

            if (sd > 0)
                FD_SET(sd, &readfds);

            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        if (FD_ISSET(listen_sock, &readfds))
        {
            sin_size = sizeof(struct sockaddr_in);
            if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection , socket fd is %d , ip is : %s , port : %d \n", conn_sock, inet_ntoa(client.sin_addr), ntohs(client.sin_port));

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = conn_sock;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds))
            {
                char buff[BUFF_SIZE];
                int bytes_received = recv(sd, buff, BUFF_SIZE, 0);

                if (bytes_received == 0)
                {
                    getpeername(sd, (struct sockaddr *)&client, &sin_size);
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                    for (int j = 0; j < num_accounts; j++)
                    {
                        if (accounts[j].client_id == sd)
                        {
                            accounts[j].client_id = -1;
                            accounts[j].logged_in = 0;
                            printf("Account %s has logged out\n", accounts[j].userID);
                            break;
                        }
                    }
                    close(sd);
                    client_sockets[i] = 0;
                }
                else
                {
                    Account *account = get_logged_in_account(sd);
                    if (account != NULL)
                    {
                        buff[bytes_received] = '\0';
                        // This client has logged in, treat the received string as a normal message.
                        printf("Received message from %s: %s\n", account->userID, buff);
                        char reply[BUFF_SIZE * 2 + 1];
                        sprintf(reply, "Server received message: %s", buff);
                        send(sd, reply, strlen(reply), 0);
                    }
                    else
                    {
                        buff[bytes_received] = '\0';
                        if (strlen(buff) > 0)
                        {
                            handle_login(buff, sd, account);
                            break;
                        }
                    }
                }
            }
        }
    }

    close(listen_sock);
    return 0;
}
