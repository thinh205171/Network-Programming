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

#define BUFF_SIZE 10000
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

void send_file(int socket)
{
    char file_name[BUFF_SIZE];
    printf("Nhập đường dẫn tới file: ");
    fgets(file_name, BUFF_SIZE, stdin);
    file_name[strcspn(file_name, "\n")] = 0;

    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Không thể mở file. Vui lòng kiểm tra lại đường dẫn.\n");
        send(socket, "error", strlen(file_name), 0);
        return;
    }

    // Gửi tên file
    send(socket, file_name, strlen(file_name), 0);

    char buffer[BUFF_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFF_SIZE, file)) > 0)
    {
        send(socket, buffer, bytes_read, 0);
    }

    fclose(file);
    printf("Đã gửi nội dung file thành công.\n");
}

void send_image(int socket)
{
    char file_name[BUFF_SIZE];
    printf("Nhập đường dẫn tới file ảnh: ");
    fgets(file_name, BUFF_SIZE, stdin);
    file_name[strcspn(file_name, "\n")] = 0;

    FILE *file = fopen(file_name, "rb");
    if (file == NULL)
    {
        printf("Không thể mở file. Vui lòng kiểm tra lại đường dẫn.\n");
        send(socket, "error", strlen(file_name), 0);
        return;
    }

    // Gửi tên file
    // send(socket, file_name, strlen(file_name), 0);
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send file size
    send(socket, &file_size, sizeof(file_size), 0);

    char buffer[BUFF_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFF_SIZE, file)) > 0)
    {
        send(socket, buffer, bytes_read, 0);
    }

    fclose(file);
    printf("Đã gửi nội dung file ảnh thành công.\n");
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
    int bytes_sent, bytes_received;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError! Can not connect to the server! Client exits immediately! ");
        return 0;
    }

    bool is_logged_in = false;
    int attempts = 0;

    while (1)
    {
        if (!is_logged_in)
        {
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
        }
        else
        {
            printf("\nMENU\n");
            printf("-----------------------------------\n");
            printf("1. Gửi một ảnh\n");
            printf("2. Gửi nội dung một file\n");
            printf("3. Exit\n");
            printf("-----------------------------------\n");
            printf("Nhập lựa chọn của bạn: ");
            int choice;
            scanf("%d", &choice);
            getchar(); // Xóa bộ nhớ đệm

            switch (choice)
            {
            case 1:
                send(client_sock, "image", sizeof("image"), 0);
                send_image(client_sock);
                break;
            case 2:
                send(client_sock, "file", sizeof("file"), 0);
                send_file(client_sock);
                break;
            case 3:
                close(client_sock);
                return 0;
            default:
                printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
            }
        }
    }

    close(client_sock);
    return 0;
}
