#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Định nghĩa kích thước tin nhắn
#define MSG_SIZE 1024

// Khai báo biến
int sockfd;
struct sockaddr_in server_addr, client1_addr;

int main()
{
    // Khởi tạo socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Không thể tạo socket\n");
        return 0;
    }

    // Thiết lập địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8080);

    // Thiết lập địa chỉ client1
    client1_addr.sin_family = AF_INET;
    client1_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Địa chỉ IP loopback
    client1_addr.sin_port = htons(8081);

    while (1)
    {
        // Nhập tin nhắn từ Client 1
        char msg[MSG_SIZE];
        printf("Nhập tin nhắn từ Client 1: ");
        fgets(msg, MSG_SIZE, stdin);

        if (strcmp(msg, "\n") == 0)
        {
            sendto(sockfd, "Exit", strlen("Exit"), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
            break;
        }

        // Gửi tin nhắn đến Server
        sendto(sockfd, (const char *)msg, strlen(msg), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));

        // Nhận tin nhắn từ client 2 qua server
        char buffer_alpha[MSG_SIZE];
        int len, n;
        len = sizeof(server_addr);
        n = recvfrom(sockfd, (char *)buffer_alpha, MSG_SIZE, 0, (struct sockaddr *)&server_addr, &len);
        buffer_alpha[n] = '\0';
        printf("Tin nhắn từ Client 2: %s\n", buffer_alpha);

        if (strcmp(buffer_alpha, "Exit") == 0)
        {
            break;
        }
    }

    // Đóng socket
    close(sockfd);

    return 0;
}
