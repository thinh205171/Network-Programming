#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

// Định nghĩa kích thước tin nhắn
#define MSG_SIZE 1024

// Khai báo biến
int sockfd;
struct sockaddr_in server_addr, client1_addr, client2_addr;

// // Hàm xử lý tin nhắn
// void process_message(char *msg, char *letters, char *numbers)
// {
//     int i, j, k;
//     j = k = 0;
//     for (i = 0; msg[i] != '\0'; i++)
//     {
//         if (isalpha(msg[i])) // Nếu ký tự là chữ cái
//         {
//             letters[j++] = msg[i]; // Thêm vào xâu letters
//         }
//         else if (isdigit(msg[i])) // Nếu ký tự là chữ số
//         {
//             numbers[k++] = msg[i]; // Thêm vào xâu numbers
//         }
//         // else // Nếu ký tự không phải là chữ cái hoặc chữ số
//         // {
//         //     printf("Lỗi: Tin nhắn chứa ký tự không phải là chữ cái hoặc chữ số.\n");
//         //     letters[0] = '\0';
//         //     numbers[0] = '\0';
//         //     return;
//         // }
//     }
//     letters[j] = '\0'; // Kết thúc xâu letters
//     numbers[k] = '\0'; // Kết thúc xâu numbers
// }

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
    client1_addr.sin_port = htons(8081);                   // Thay thế PORT_CỦA_CLIENT2 bằng cổng của client2

    // Thiết lập địa chỉ client2
    client2_addr.sin_family = AF_INET;
    client2_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Địa chỉ IP loopback
    client2_addr.sin_port = htons(8082);                   // Thay thế PORT_CỦA_CLIENT2 bằng cổng của client2

    // Liên kết socket với địa chỉ server
    int bind_status = bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_status < 0)
    {
        printf("Lỗi trong quá trình liên kết\n");
        return 0;
    }

    printf("Bạn đã khởi chạy server thành công. Tiêp theo hãy khởi chạy client 2 trước khi khởi chạy client 1.\nĐang lắng nghe tin nhắn từ client và client 2, nhưng client 1 sẽ là người bắt đầu cuộc trò chuyện...\n");

    while (1)
    {
        // Nhận tin nhắn từ Client 1
        char buffer[MSG_SIZE], letters[MSG_SIZE], numbers[MSG_SIZE];
        int len, n, send_status;
        len = sizeof(client1_addr);
        n = recvfrom(sockfd, (char *)buffer, MSG_SIZE, 0, (struct sockaddr *)&client1_addr, &len);
        buffer[n] = '\0';
        printf("Tin nhắn từ Client 1: %s\n", buffer);

        // // Xử lý tin nhắn từ Client 1
        // process_message(buffer, letters, numbers);
        // if (letters[0] == '\0' && numbers[0] == '\0') // Nếu có lỗi trong tin nhắn
        // {
        //     // Gửi lại tin nhắn lỗi đến Client 1
        //     send_status = sendto(sockfd, "Tin nhắn của bạn chứa ký tự không phải là chữ cái hoặc chữ số.", strlen("Tin nhắn của bạn chứa ký tự không phải là chữ cái hoặc chữ số."), 0, (const struct sockaddr *)&client1_addr, sizeof(client1_addr));
        //     if (send_status < 0)
        //     {
        //         printf("Lỗi trong quá trình gửi tin nhắn lỗi từ server sang Client 1\n");
        //     }
        // }
        // else
        // {
            if (strcmp(buffer, "Exit") == 0)
            {
                send_status = sendto(sockfd, "Exit", strlen("Exit"), 0, (const struct sockaddr *)&client2_addr, sizeof(client2_addr));
                break;
            }

            // Chuyển tiếp tin nhắn từ Client 1 sang Client 2
            int send_status = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&client2_addr, sizeof(client2_addr));
            if (send_status < 0)
            {
                printf("Lỗi trong quá trình gửi tin nhắn từ Client 1 sang Client 2\n");
            }
        // }

        // Nhận tin nhắn từ Client 2
        len = sizeof(client2_addr);
        n = recvfrom(sockfd, (char *)buffer, MSG_SIZE, 0, (struct sockaddr *)&client2_addr, &len);
        buffer[n] = '\0';
        printf("Tin nhắn từ Client 2: %s\n", buffer);

        // // Xử lý tin nhắn từ Client 2
        // process_message(buffer, letters, numbers);
        // if (letters[0] == '\0' && numbers[0] == '\0') // Nếu có lỗi trong tin nhắn
        // {
        //     // Gửi lại tin nhắn lỗi đến Client 2
        //     int send_status = sendto(sockfd, "Tin nhắn của bạn chứa ký tự không phải là chữ cái hoặc chữ số.", strlen("Tin nhắn của bạn chứa ký tự không phải là chữ cái hoặc chữ số."), 0, (const struct sockaddr *)&client2_addr, sizeof(client2_addr));
        //     if (send_status < 0)
        //     {
        //         printf("Lỗi trong quá trình gửi tin nhắn lỗi từ server sang Client 1\n");
        //     }
        // }
        // else
        // {
            if (strcmp(buffer, "Exit") == 0)
            {
                send_status = sendto(sockfd, "Exit", strlen("Exit"), 0, (const struct sockaddr *)&client1_addr, sizeof(client1_addr));
                break;
            }

            // Chuyển tiếp tin nhắn từ Client 2 sang Client 1
            send_status = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&client1_addr, sizeof(client1_addr));
            if (send_status < 0)
            {
                printf("Lỗi trong quá trình gửi tin nhắn từ Client 2 sang Client 1\n");
            }
        // }
    }

    // Đóng socket
    close(sockfd);

    return 0;
}
