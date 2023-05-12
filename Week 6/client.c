#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFF_SIZE 8192

void send_string(int client_sock);
void send_file(int client_sock);

int client_sock, exit_flag = 0;
char buff[BUFF_SIZE];
struct sockaddr_in server_addr; /* server's address information */
int msg_len, bytes_sent, bytes_received;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./client IPAddress PortNumber\n");
        return 0;
    }

    int SERVER_PORT = atoi(argv[2]);
    char *SERVER_ADDR = argv[1];

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately! ");
        return 0;
    }

    // Hiển thị menu cho người dùng
    while (1)
    {
        printf("\nMENU\n");
        printf("-----------------------------------\n");
        printf("1. Gửi xâu bất kỳ\n");
        printf("2. Gửi nội dung một file\n");
        printf("-----------------------------------\n");
        printf("Nhập lựa chọn của bạn: ");
        int choice;
        scanf("%d", &choice);
        getchar(); // Xóa bộ nhớ đệm

        switch (choice)
        {
        case 1:
            bytes_sent = send(client_sock, "string", strlen("string"), 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed!\n");
                return 0;
            }
            send_string(client_sock);
            if (exit_flag)
                return 0;
            break;
        case 2:
            bytes_sent = send(client_sock, "file", strlen("file"), 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed!\n");
                return 0;
            }
            send_file(client_sock);
            break;
        default:
            printf("Lựa chọn không hợp lệ. Vui lòng thử lại.\n");
        }
    }

    close(client_sock);
    return 0;
}

void send_string(int client_sock)
{
    // send message
    printf("Insert string to send: ");
    memset(buff, '\0', (strlen(buff) + 1));
    fgets(buff, BUFF_SIZE, stdin);
    buff[strcspn(buff, "\n")] = '\0';

    msg_len = strlen(buff);
    if (msg_len <= 1)
    {
        strcpy(buff, "exit");
        send(client_sock, buff, strlen(buff), 0);
        exit_flag = 1;
    }

    bytes_sent = send(client_sock, buff, msg_len, 0);
    if (bytes_sent <= 0)
    {
        printf("\nConnection closed!\n");
        return;
    }
    char letters[BUFF_SIZE] = {0};
    char digits[BUFF_SIZE] = {0};
    int letter_index = 0, digit_index = 0;
    int is_reading_digits = 0;

    // receive echo letter reply
    bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
    if (bytes_received <= 0)
    {
        printf("\nError!Cannot receive data from sever!\n");
        return;
    }

    buff[bytes_received] = '\0';

    if (strcmp(buff, "Error! Invalid input") != 0)
    {
        for (int i = 0; i < bytes_received; ++i)
        {
            if (buff[i] == '\n')
            {
                is_reading_digits = 1;
            }
            else if (!is_reading_digits)
            {
                letters[letter_index++] = buff[i];
            }
            else
            {
                digits[digit_index++] = buff[i];
            }
        }

        printf("\nLetter reply from server: %s\n", letters);
        printf("Digit reply from server: %s\n\n", digits);
    }
    else
    {
        printf("Error! Invalid input\n\n");
    }
}

void send_file(int client_sock)
{
    char file_name[BUFF_SIZE];
    printf("Nhập đường dẫn tới file: ");
    fgets(file_name, BUFF_SIZE, stdin);
    file_name[strcspn(file_name, "\n")] = 0;

    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Không thể mở file. Vui lòng kiểm tra lại đường dẫn.\n");
        send(client_sock, "error", strlen(file_name), 0);
        return;
    }

    // Gửi tên file
    send(client_sock, file_name, strlen(file_name), 0);

    // Lấy kích thước file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Gửi kích thước file
    send(client_sock, &file_size, sizeof(file_size), 0);

    char buffer[BUFF_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFF_SIZE, file)) > 0)
    {
        send(client_sock, buffer, bytes_read, 0);
    }

    fclose(file);
    printf("Đã gửi nội dung file thành công.\n");

    // Nhận thông báo từ server
    char server_response[BUFF_SIZE];
    int response_received = recv(client_sock, server_response, BUFF_SIZE - 1, 0);
    if (response_received > 0)
    {
        server_response[response_received] = '\0';
        printf("Server response: %s\n", server_response);
        memset(server_response, 0, response_received);
    }
    else
    {
        printf("Không nhận được phản hồi từ server.\n");
    }
}