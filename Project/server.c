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
#include "product.h"
#include "demnguoc.h"

#define BACKLOG 20
#define BUFF_SIZE 1024
#define MAX_CLIENTS 100
#define MAX_PRODUCTS 100

Product products[MAX_PRODUCTS];
int numProducts = 0;

extern Account accounts[MAX_ACCOUNTS];
extern int num_accounts;

void readProducts(Product products[], int maxProducts, const char *filename, int *numProducts);

void send_data(int socket, const char *data)
{
    int bytes_sent = send(socket, data, strlen(data), 0);
    if (bytes_sent == -1)
    {
        perror("Error sending data");
    }
}

void receive_data(int socket, char *buffer, int buffer_size)
{
    int bytes_received = recv(socket, buffer, buffer_size - 1, 0);
    if (bytes_received == -1)
    {
        perror("Error receiving data");
    }
    else if (bytes_received == 0)
    {
        // Client đã ngắt kết nối
        printf("Client disconnected\n");
    }
    else
    {
        buffer[bytes_received] = '\0'; // Đảm bảo kết thúc chuỗi
        printf("Received data from client: %s\n", buffer);
    }
}

void save_account_to_file(const char *username, const char *password)
{
    FILE *file = fopen("account.txt", "a");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return;
    }

    fprintf(file, "%s %s %d %d\n", username, password, 1, 100);

    fclose(file);
}

void handle_signup(const char *data, int client_id)
{
    char username[BUFF_SIZE * 3];
    char password[BUFF_SIZE * 3];

    sscanf(data, "%s %s", username, password);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (accounts[i].client_id == client_id)
        {
            send_data(client_id, "This account is already logged in.\n");
            return;
        }

        if (strcmp(accounts[i].userID, username) == 0)
        {
            send_data(client_id, "Account already exists.\n");
            return;
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (accounts[i].client_id == 0)
        {
            accounts[i].client_id = client_id;
            accounts[i].logged_in = 1;
            strcpy(accounts[i].userID, username);
            strcpy(accounts[i].password, password);
            accounts[i].point = 100; // Gán giá trị mặc định cho điểm

            printf("New account created: %s\n", accounts[i].userID);
            send_data(client_id, "Account created successfully.\n");

            // Lưu thông tin tài khoản vào tệp tin
            save_account_to_file(username, password);
            return;
        }
    }

    send_data(client_id, "Maximum number of accounts reached.\n");
}

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
                    int selectedProducts[10];
                    if (account != NULL)
                    {
                        char productsInfo[BUFF_SIZE * 6 + 1];
                        productsInfo[BUFF_SIZE * 6 + 1] = '\0';
                        buff[bytes_received] = '\0';
                        // This client has logged in, treat the received string as a normal message.
                        printf("Received message from %s: %s\n", account->userID, buff);
                        if (strcmp(buff, "ready") == 0)
                        {
                            char reply[BUFF_SIZE * 2 + 1];
                            sprintf(reply, "%s", "Bắt đầu trò chơi (type 'grocery' to get product info)");
                            send_data(sd, reply);
                            readProducts(products, MAX_PRODUCTS, "product.txt", &numProducts);
                            for (int i = 0; i < 5; i++) // Chọn ngẫu nhiên 3 sản phẩm để chơi
                            {
                                selectedProducts[i] = rand() % numProducts;
                                for (int j = 0; j < i; j++) // Kiểm tra giá trị đã được chọn trước đó
                                {
                                    if (selectedProducts[i] == selectedProducts[j])
                                    {
                                        i--; // Giá trị đã trùng lặp, chọn lại giá trị khác
                                        break;
                                    }
                                }
                            }
                            // buff[bytes_received] = '\0';
                        }
                        else if (strcmp(buff, "grocery") == 0)
                        {
                            printf("Các sản phẩm để chơi trò đếm ngược:\n");
                            for (int i = 0; i < 5; i++)
                            {
                                printf("Sản phẩm %d:\n", i + 1);
                                printf("Tên: %s\n", products[selectedProducts[i]].name);
                                printf("Giá: %d\n", products[selectedProducts[i]].price);
                                char productInfo[BUFF_SIZE * 2 + 1];
                                sprintf(productInfo, "Sản phẩm %d:\nTên: %s\nGiá: %d\n", i + 1, products[selectedProducts[i]].name, products[selectedProducts[i]].price);
                                strcat(productsInfo, productInfo);
                            }
                            // Gửi chuỗi productsInfo tới client
                            send_data(sd, productsInfo);
                            productsInfo[0] = '\0';
                        }
                        else if (strcmp(buff, "logout") == 0)
                        {
                            send_data(sd, "See you again!!\n");
                        }
                        else
                        {
                            send_data(sd, "Invalid command\n");
                        }
                    }
                    else
                    {
                        buff[bytes_received] = '\0';

                        // if (strcmp(buff, "signup") == 0)
                        // {
                        //     handle_signup(buff, sd);
                        //     break;
                        // }
                        // if (strcmp(buff, "login") == 0)
                        if(strlen(buff) > 0)
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
