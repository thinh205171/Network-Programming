#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_USERNAME_LENGTH 100
#define MAX_PASSWORD_LENGTH 100
#define MAX_RESPONSE_LENGTH 100

int main(int argc, char *argv[])
{
    // Kiểm tra đủ số lượng tham số dòng lệnh
    if (argc != 3)
    {
        printf("Sử dụng: %s IPAddress PortNumber\n", argv[0]);
        return 1;
    }

    // Đọc địa chỉ IP và số hiệu cổng từ tham số dòng lệnh
    char *ip_address = argv[1];
    int port_number = atoi(argv[2]);

    // Tạo socket
    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock == -1)
    {
        perror("Lỗi tạo socket");
        return 1;
    }

    // Cấu hình địa chỉ server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);

    if (inet_pton(AF_INET, ip_address, &(server_addr.sin_addr)) <= 0)
    {
        perror("Lỗi chuyển đổi địa chỉ IP");
        return 1;
    }

    // Lặp lại chức năng cho đến khi nhập xâu rỗng
    while (1)
    {
        // Nhập tên đăng nhập từ bàn phím
        char username[MAX_USERNAME_LENGTH];
        printf("Nhập tên đăng nhập (hoặc rỗng để thoát): ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0';

        // Kiểm tra xem người dùng có muốn thoát hay không
        if (strlen(username) == 0)
        {
            break;
        }

        // Gửi tên đăng nhập cho server
        sendto(client_sock, username, strlen(username), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        // Nhận kết quả đăng nhập từ server
        char login_status[MAX_RESPONSE_LENGTH];
        socklen_t server_addr_len = sizeof(server_addr);
        ssize_t login_status_length = recvfrom(client_sock, login_status, sizeof(login_status), 0,
                                               (struct sockaddr *)&server_addr, &server_addr_len);

        if (login_status_length == -1)
        {
            perror("Lỗi nhận dữ liệu");
            continue;
        }

        // Hiển thị kết quả đăng nhập
        printf("Kết quả đăng nhập: %s\n", login_status);

        // Nếu đăng nhập thành công
        if (strcmp(login_status, "OK") == 0)
        {
            while (1)
            {
                // Nhập yêu cầu từ bàn phím
                char request[MAX_RESPONSE_LENGTH];
                printf("Nhập yêu cầu (changepassword hoặc bye): ");
                fgets(request, sizeof(request), stdin);
                request[strcspn(request, "\n")] = '\0';

                // Gửi yêu cầu đến server
                sendto(client_sock, request, strlen(request), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
                // Kiểm tra xem người dùng có muốn đăng xuất hay không
                if (strcmp(request, "bye") == 0)
                {
                    // Nhận kết quả đăng xuất từ server
                    char logout_status[MAX_RESPONSE_LENGTH];
                    ssize_t logout_status_length = recvfrom(client_sock, logout_status, sizeof(logout_status), 0,
                                                            (struct sockaddr *)&server_addr, &server_addr_len);

                    if (logout_status_length == -1)
                    {
                        perror("Lỗi nhận dữ liệu");
                        continue;
                    }

                    // Hiển thị kết quả đăng xuất và thoát khỏi vòng lặp
                    printf("Kết quả đăng xuất: %s\n", logout_status);
                    break;
                }
                else if (strcmp(request, "changepassword") == 0)
                {
                    // Nhập mật khẩu mới từ bàn phím
                    char new_password[MAX_PASSWORD_LENGTH];
                    printf("Nhập mật khẩu mới: ");
                    fgets(new_password, sizeof(new_password), stdin);
                    new_password[strcspn(new_password, "\n")] = '\0';

                    // Gửi mật khẩu mới đến server
                    sendto(client_sock, new_password, strlen(new_password), 0,
                           (struct sockaddr *)&server_addr, sizeof(server_addr));

                    // Nhận mật khẩu mới đã mã hóa từ server
                    char encrypted_password_alpha[MAX_PASSWORD_LENGTH];
                    char encrypted_password_digit[MAX_PASSWORD_LENGTH];
                    ssize_t encrypted_password_alpha_length = recvfrom(client_sock, encrypted_password_alpha, sizeof(encrypted_password_alpha), 0,
                                                                       (struct sockaddr *)&server_addr, &server_addr_len);
                    ssize_t encrypted_password_digit_length = recvfrom(client_sock, encrypted_password_digit, sizeof(encrypted_password_digit), 0,
                                                                       (struct sockaddr *)&server_addr, &server_addr_len);

                    if (encrypted_password_alpha_length == -1 || encrypted_password_digit_length == -1)
                    {
                        perror("Lỗi nhận dữ liệu");
                        continue;
                    }

                    // Hiển thị mật khẩu mới đã mã hóa
                    printf("Mật khẩu mới đã mã hóa: %s%s\n", encrypted_password_alpha, encrypted_password_digit);
                }
                else
                {
                    printf("Yêu cầu không hợp lệ\n");
                }
            }
        }
    }

    // Đóng socket
    close(client_sock);

    return 0;
}
