#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 5550  /* Port that will be opened */
#define BACKLOG 20 /* Number of allowed connections */
#define BUFF_SIZE 1024

#define MAX_USERS 1001

typedef struct
{
    char username[100];
    char password[100];
    int status;
    int login_attempts;
    int logged_in;
    int locked;
} user_t;

int n_users = 0;
user_t users[MAX_USERS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void load_users_from_file()
{
    FILE *fp = fopen("Account.txt", "r");
    if (fp == NULL)
    {
        perror("fopen");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        char username[100], password[100];
        int status;
        if (sscanf(line, "%s %s %d", username, password, &status) != 3)
        {
            fprintf(stderr, "Invalid line in account.txt: %s", line);
            continue;
        }

        if (n_users >= MAX_USERS)
        {
            fprintf(stderr, "Too many users in account.txt");
            continue;
        }

        strncpy(users[n_users].username, username, sizeof(users[n_users].username));
        strncpy(users[n_users].password, password, sizeof(users[n_users].password));
        users[n_users].status = status;
        users[n_users].login_attempts = 0;
        users[n_users].logged_in = 0;
        n_users++;
    }

    fclose(fp);
}

int authenticate_user(char *username, char *password)
{
    pthread_mutex_lock(&mutex);
    int i;
    for (i = 0; i < n_users; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            if (users[i].locked == 1)
            {
                pthread_mutex_unlock(&mutex);
                return -1; /* Account is blocked */
            }
            else if (strcmp(users[i].password, password) == 0)
            {
                users[i].login_attempts++;
                users[i].logged_in = 1;
                pthread_mutex_unlock(&mutex);
                return 1; /* Authentication successful */
            }
            else
            {
                users[i].login_attempts++;
                if (users[i].login_attempts >= 3)
                {
                    users[i].locked = 1; /* Lock account */
                    pthread_mutex_unlock(&mutex);
                    return 0; /* Authentication failed */
                }
                pthread_mutex_unlock(&mutex);
                return 0; /* Authentication failed */
            }
        }
    }
    pthread_mutex_unlock(&mutex);

    return 0; /* Authentication failed */
}

void print_user_info(char *username)
{
    int i;
    for (i = 0; i < n_users; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            printf("Username: %s\n", users[i].username);
            printf("Password: %s\n", users[i].password);
            printf("Status: %d\n", users[i].status);
            printf("Login Attempts: %d\n", users[i].login_attempts);
            return;
        }
    }
    printf("User not found or wrong password many times\n");
}

void logout_user(char *username)
{
    int i;
    for (i = 0; i < n_users; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            users[i].logged_in = 0;
            printf("User %s logged out.\n", username);

            // Đặt trạng thái khóa tài khoản thành 0
            if (users[i].login_attempts >= 3)
            {
                users[i].locked = 0;
                users[i].login_attempts = 0;
            }
            return;
        }
    }
    printf("User not found or wrong password many times\n");
}

void *echo(void *arg);

int main()
{
    int listenfd, *connfd;
    struct sockaddr_in server;  /* server's address information */
    struct sockaddr_in *client; /* client's address information */
    socklen_t sin_size;
    pthread_t tid;

    load_users_from_file();

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

    if (bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    sin_size = sizeof(struct sockaddr_in);
    client = malloc(sin_size);
    while (1)
    {
        connfd = malloc(sizeof(int));
        if ((*connfd = accept(listenfd, (struct sockaddr *)client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa(client->sin_addr)); /* prints client's IP */

        /* For each client, spawns a thread, and the thread handles the new client */
        pthread_create(&tid, NULL, &echo, connfd);
    }

    close(listenfd);
    return 0;
}

void *echo(void *arg)
{
    int connfd;
    int bytes_sent, bytes_received;
    char buff[BUFF_SIZE + 1];

    connfd = *((int *)arg);
    free(arg);
    pthread_detach(pthread_self());

    // Tên người dùng hiện tại của kết nối
    char current_username[100] = "";

    while (1)
    {
        bytes_received = recv(connfd, buff, BUFF_SIZE, 0); // Blocking
        if (bytes_received < 0)
            perror("\nError: ");
        else if (bytes_received == 0)
            printf("Connection closed.\n");
        else
        {
            buff[bytes_received] = '\0';

            char username[100], password[100];
            sscanf(buff, "%s %s", username, password);

            if (strcmp(username, "logout") == 0)
            {
                logout_user(current_username); // Đăng xuất trên tất cả các kết nối
                snprintf(buff, BUFF_SIZE, "Logout successful\n");
                bytes_sent = send(connfd, buff, strlen(buff), 0);
                if (bytes_sent < 0)
                    perror("\nError: ");
                break; // Thoát khỏi vòng lặp while để đóng kết nối
            }
            if (strcmp(username, "exit") == 0)
            {
                logout_user(current_username); // Đăng xuất trên tất cả các kết nối
                snprintf(buff, BUFF_SIZE, "Exit program.\n");
                bytes_sent = send(connfd, buff, strlen(buff), 0);
                if (bytes_sent < 0)
                    perror("\nError: ");
                break; // Thoát khỏi vòng lặp while để đóng kết nối
            }
            else
            {
                int auth_result = authenticate_user(username, password);
                if (auth_result == -1)
                    snprintf(buff, BUFF_SIZE, "Wrong password 3 times.\n");
                else if (auth_result == 1)
                {
                    snprintf(buff, BUFF_SIZE, "Authentication successful.\n");
                    print_user_info(username);
                    // Lưu tên người dùng hiện tại của kết nối
                    strncpy(current_username, username, sizeof(current_username));
                }
                else
                    snprintf(buff, BUFF_SIZE, "Authentication failed.\n");
            }

            bytes_sent = send(connfd, buff, strlen(buff), 0);
            if (bytes_sent < 0)
                perror("\nError: ");
        }
    }

    close(connfd);
    return NULL;
}
