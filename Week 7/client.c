#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 1024

int main()
{
    int client_sock;
    char buff[BUFF_SIZE + 1];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received;
    char username[100], password[100];

    int logged_in = 0;
    int login_attempts = 0;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError! Không thể kết nối với máy chủ. Chương trình client sẽ kết thúc!");
        return 0;
    }

    while (1)
    {
        if (!logged_in)
        {
            if (login_attempts == 3)
            {
                snprintf(buff, BUFF_SIZE, "exit");
                msg_len = strlen(buff);
                bytes_sent = send(client_sock, buff, msg_len, 0);
                if (bytes_sent < 0)
                {
                    perror("\nError: ");
                    break;
                }
                close(client_sock); // Đóng kết nối
                return 0;           // Kết thúc chương trình
            }

            printf("Tên người dùng: ");
            scanf("%s", username);

            printf("Mật khẩu: ");
            scanf("%s", password);

            snprintf(buff, BUFF_SIZE, "%s %s", username, password);
            msg_len = strlen(buff);

            bytes_sent = send(client_sock, buff, msg_len, 0);
            if (bytes_sent < 0)
            {
                perror("\nError: ");
                break;
            }

            bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
            if (bytes_received < 0)
            {
                perror("\nError: ");
                break;
            }
            else if (bytes_received == 0)
                printf("Kết nối đã đóng.\n");

            buff[bytes_received] = '\0';
            printf("Phản hồi từ máy chủ: %s", buff);

            if (strncmp(buff, "Authentication failed", strlen("Authentication failed")) == 0)
            {
                login_attempts++;
                if (login_attempts < 3)
                    printf("Nhập sai. Vui lòng thử lại.\n");
                continue; // Thử lại quá trình đăng nhập
            }

            if (strncmp(buff, "Authentication successful", strlen("Authentication successful")) == 0)
            {
                logged_in = 1;
                printf("Đăng nhập thành công!\n");
            }

            // if (strncmp(buff, "Wrong password 3 times.", strlen("Wrong password 3 times.")) == 0)
            // {
            //     snprintf(buff, BUFF_SIZE, "exit");
            //     msg_len = strlen(buff);
            //     bytes_sent = send(client_sock, buff, msg_len, 0);
            //     if (bytes_sent < 0)
            //     {
            //         perror("\nError: ");
            //         break;
            //     }
            //     close(client_sock); // Đóng kết nối
            //     return 0;           // Kết thúc chương trình
            // }

            while (logged_in)
            {
                printf("Bạn có muốn đăng xuất? (Y/N): ");
                char choice;
                scanf(" %c", &choice);
                if (choice == 'Y' || choice == 'y')
                {
                    // Thực hiện đăng xuất
                    printf("Đăng xuất thành công.\n");
                    logged_in = 0;

                    // Gửi thông báo đóng kết nối tới server
                    snprintf(buff, BUFF_SIZE, "logout");
                    msg_len = strlen(buff);
                    bytes_sent = send(client_sock, buff, msg_len, 0);
                    if (bytes_sent < 0)
                    {
                        perror("\nError: ");
                        break;
                    }

                    // Đóng kết nối socket phía client
                    close(client_sock);

                    return 0; // Thoát khỏi vòng lặp và kết thúc chương trình
                }
            }
        }
    }

    // Đóng kết nối
    close(client_sock);

    return 0;
}
