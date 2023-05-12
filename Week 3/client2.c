#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Định nghĩa kích thước tin nhắn
#define MSG_SIZE 1024

// Khai báo biến
int sockfd;
struct sockaddr_in server_addr;

int main()
{
    // Khởi tạo socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Không thể tạo socket\n");
        return 0;
    }

    // Thiết lập địa chỉ server cho server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Địa chỉ IP của server
    server_addr.sin_port = htons(8080);                   // Cổng của server

    // Thiết lập địa chỉ client2
    struct sockaddr_in client2_addr;
    client2_addr.sin_family = AF_INET;
    client2_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Địa chỉ IP loopback
    client2_addr.sin_port = htons(8082);                   // Thay thế PORT_CỦA_CLIENT2 bằng cổng của client2

    // Liên kết socket với địa chỉ server
    int bind_status = bind(sockfd, (const struct sockaddr *)&client2_addr, sizeof(client2_addr));
    if (bind_status < 0)
    {
        printf("Lỗi trong quá trình liên kết\n");
        return 0;
    }

    printf("Chờ server gửi tin nhắn đầu tiên từ client 1...");

    while (1)
    {
        // Nhận tin nhắn từ client 1 qua server
        char buffer_alpha[MSG_SIZE];
        int len, n;
        len = sizeof(server_addr);
        n = recvfrom(sockfd, (char *)buffer_alpha, MSG_SIZE, 0, (struct sockaddr *)&server_addr, &len);
        buffer_alpha[n] = '\0';
        printf("Tin nhắn từ Client 1: %s\n", buffer_alpha);

        if (strcmp(buffer_alpha, "Exit") == 0)
        {
            break;
        }

        // Nhập tin nhắn từ Client 2
        char msg[MSG_SIZE];
        printf("Nhập tin nhắn từ Client 2: ");
        fgets(msg, MSG_SIZE, stdin);

        if (strcmp(msg, "\n") == 0)
        {
            sendto(sockfd, "Exit", strlen("Exit"), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
            break;
        }
        // Gửi tin nhắn đến Server
        sendto(sockfd, (const char *)msg, strlen(msg), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    // char buffer_num[MSG_SIZE];
    // n = recvfrom(sockfd, (char *)buffer_num, MSG_SIZE, 0, (struct sockaddr *)&server_addr, &len);
    // buffer_num[n] = '\0';
    // printf("Tin nhắn số từ Client 1: %s\n", buffer_num);

    // Đóng socket
    close(sockfd);

    return 0;
}
