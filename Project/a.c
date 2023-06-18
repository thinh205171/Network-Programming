#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

#define BUFF_SIZE 1024
#define MAX_ATTEMPTS 3

char *trimwhitespace(char *str)
{
    char *end;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    end[1] = '\0';

    return str;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    int SERVER_PORT = atoi(argv[2]);
    char *SERVER_ADDR = argv[1];

    int client_sock;
    char buff[BUFF_SIZE + 1];
    struct sockaddr_in server_addr;
    int msg_len, bytes_sent, bytes_received;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError! Can not connect to server! Client exiting immediately!");
        return 0;
    }

    bool is_logged_in = false;
    int attempts = 0;
    bool is_game_started = false;
    int choice = 0;

    while (1)
    {
        if (!is_logged_in)
        {
            printf("\n--------------");
            printf("\n1. Sign up");
            printf("\n2. Log in");
            printf("\n3. Exit");
            printf("\n--------------");
            printf("\nPlease enter your choice: ");
            scanf("%d", &choice);
            getchar(); // Đọc ký tự new line

            switch (choice)
            {
            case 1:
                // Xử lý Sign up
                break;
            case 2:
                char username[BUFF_SIZE], password[BUFF_SIZE];

                printf("Username: ");
                fgets(username, BUFF_SIZE, stdin);
                username[strlen(username) - 1] = '\0';

                printf("Password: ");
                fgets(password, BUFF_SIZE, stdin);
                password[strlen(password) - 1] = '\0';

                char login_info[BUFF_SIZE * 2 + 1];
                sprintf(login_info, "%s %s", username, password);

                bytes_sent = send(client_sock, login_info, strlen(login_info), 0);
                if (bytes_sent < 0)
                    perror("\nError: ");

                bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
                if (bytes_received < 0)
                    perror("\nError: ");
                else if (bytes_received == 0)
                    printf("Connection closed.\n");

                buff[bytes_received] = '\0';
                printf("%s\n", buff);

                // Kiểm tra nếu đăng nhập thành công
                if (strcmp(buff, "Login success") == 0)
                {
                    is_logged_in = true;
                }
                else
                {
                    attempts++;
                    printf("Cannot log in, attempt %d/%d\n", attempts, MAX_ATTEMPTS);
                    if (attempts >= MAX_ATTEMPTS)
                    {
                        printf("Max login attempts exceeded.\n");
                        close(client_sock);
                        return 0;
                    }
                }
                break;
            case 3:
                // Gửi thông báo đăng xuất tới server trước khi ngắt kết nối
                char logout_msg[] = "logout";
                send(client_sock, logout_msg, strlen(logout_msg), 0);
                close(client_sock);
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
                continue;
            }
        }
        else
        {
            if (!is_game_started)
            {
                printf("\nInsert string to send (type 'logout' to quit, type 'ready' to play): ");
            }
            else
                printf("\nInsert string to send (type 'logout' to quit): ");
            memset(buff, '\0', (strlen(buff) + 1));
            fgets(buff, BUFF_SIZE, stdin);
            buff[strlen(buff) - 1] = '\0';
            char *trimmed_buff = trimwhitespace(buff);
            if (strcmp(trimmed_buff, "ready") == 0)
            {
                is_game_started = true; // Đặt biến cờ thành true khi trò chơi bắt đầu
            }
            if (strcmp(trimmed_buff, "logout") == 0)
            {
                // Gửi thông báo đăng xuất tới server trước khi ngắt kết nối
                is_logged_in = false;
                // break;
            }

            msg_len = strlen(buff);

            bytes_sent = send(client_sock, buff, msg_len, 0);
            if (bytes_sent < 0)
                perror("\nError: ");

            bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
            if (bytes_received < 0)
                perror("\nError: ");
            else if (bytes_received == 0)
                printf("Connection closed.\n");
            buff[bytes_received] = '\0';
            printf("Reply from server:\n%s", buff);
        }
    }

    close(client_sock);
    return 0;
}
