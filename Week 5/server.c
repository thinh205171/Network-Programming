#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_USERNAME_LENGTH 100
#define MAX_PASSWORD_LENGTH 100
#define MAX_RESPONSE_LENGTH 100

typedef struct
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int status;
} Account;

// Hàm kiểm tra đăng nhập
int check_login(Account accounts[], int num_accounts, const char *username, const char *password)
{
    for (int i = 0; i < num_accounts; i++)
    {
        if (strcmp(accounts[i].username, username) == 0)
        {
            if (accounts[i].status == 0)
            {
                return -2; // Tài khoản bị khóa hoặc chưa kích hoạt
            }
            if (strcmp(accounts[i].password, password) == 0)
            {
                return 1; // Đăng nhập thành công
            }
            else
            {
                accounts[i].status--; // Giảm status sau mỗi lần nhập sai mật khẩu
                if (accounts[i].status == 0)
                {
                    return -2; // Tài khoản bị khóa sau 3 lần nhập sai mật khẩu
                }
                else
                {
                    return -1; // Mật khẩu sai
                }
            }
        }
    }
    return 0; // Tên đăng nhập không tồn tại
}

// Hàm mã hóa mật khẩu mới thành hai chuỗi: chỉ chứa chữ cái và chỉ chứa chữ số
void encrypt_password(const char *password, char *alpha_part, char *digit_part)
{
    int alpha_index = 0;
    int digit_index = 0;
    for (int i = 0; password[i] != '\0'; i++)
    {
        if (isalpha(password[i]))
        {
            alpha_part[alpha_index++] = password[i];
        }
        else if (isdigit(password[i]))
        {
            digit_part[digit_index++] = password[i];
        }
    }
    alpha_part[alpha_index] = '\0';
    digit_part[digit_index] = '\0';
}

// Hàm xử lý yêu cầu đổi mật khẩu
void change_password(Account accounts[], int num_accounts, const char *username, const char *new_password, int client_sock)
{
    for (int i = 0; i < num_accounts; i++)
    {
        if (strcmp(accounts[i].username, username) == 0)
        {
            // Kiểm tra tính hợp lệ của mật khẩu mới
            bool is_valid = true;
            for (int j = 0; new_password[j] != '\0'; j++)
            {
                if (!isalpha(new_password[j]) && !isdigit(new_password[j]))
                {
                    is_valid = false;
                    break;
                }
            }

            if (!is_valid)
            {
                send(client_sock, "Lỗi: Mật khẩu mới không hợp lệ", strlen("Lỗi: Mật khẩu mới không hợp lệ"), 0);
                return;
            }

            // Mã hóa mật khẩu mới
            char encoded_alpha[MAX_PASSWORD_LENGTH];
            char encoded_digit[MAX_PASSWORD_LENGTH];
            encrypt_password(new_password, encoded_alpha, encoded_digit);

            // Gửi mật khẩu mới đã mã hóa cho client
            send(client_sock, encoded_alpha, strlen(encoded_alpha), 0);
            send(client_sock, encoded_digit, strlen(encoded_digit), 0);
            return;
        }
    }

    send(client_sock, "Lỗi: Tài khoản không tồn tại", strlen("Lỗi: Tài khoản không tồn tại"), 0);
}

int main(int argc, char *argv[])
{
    // Kiểm tra đủ số lượng tham số dòng lệnh
    if (argc != 2)
    {
        printf("Sử dụng: %s PortNumber\n", argv[0]);
        return 1;
    }

    // Đọc dữ liệu từ file account.txt và lưu vào mảng accounts
    FILE *account_file = fopen("account.txt", "r");
    if (account_file == NULL)
    {
        perror("Lỗi đọc file");
        return 1;
    }

    Account *accounts = NULL;
    int num_accounts = 0;

    // Đọc thông tin tài khoản từ file account.txt
    char line[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 10];
    while (fgets(line, sizeof(line), account_file) != NULL)
    {
        // Cấp phát lại bộ nhớ cho mảng accounts
        accounts = (Account *)realloc(accounts, (num_accounts + 1) * sizeof(Account));
        if (accounts == NULL)
        {
            perror("Lỗi cấp phát bộ nhớ");
            fclose(account_file);
            return 1;
        }

        sscanf(line, "%s %s %d", accounts[num_accounts].username, accounts[num_accounts].password, &accounts[num_accounts].status);
        num_accounts++;
    }

    fclose(account_file);

    // Chạy ở số hiệu cổng
    int port = atoi(argv[1]);

    // Khởi tạo socket
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock == -1)
    {
        perror("Lỗi tạo socket");
        return 1;
    }

    // Cấu hình địa chỉ server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Gắn địa chỉ server vào socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Lỗi gắn địa chỉ");
        close(server_sock);
        return 1;
    }

    printf("Server đang chạy trên cổng %d\n", port);

    while (1)
    {
        // Nhận yêu cầu từ client
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        char recv_buffer[MAX_USERNAME_LENGTH];
        ssize_t recv_length = recvfrom(server_sock, recv_buffer, sizeof(recv_buffer), 0,
                                       (struct sockaddr *)&client_addr, &client_addr_len);

        if (recv_length == -1)
        {
            perror("Lỗi nhận dữ liệu");
            continue;
        }

        // Trích xuất thông tin đăng nhập từ yêu cầu nhận được
        char username[MAX_USERNAME_LENGTH];
        strcpy(username, recv_buffer);

        // Kiểm tra xem có username trong dữ liệu hay không
        bool username_found = false;
        for (int i = 0; i < num_accounts; i++)
        {
            if (strcmp(accounts[i].username, username) == 0)
            {
                username_found = true;
                break;
            }
        }

        // Nếu có username trong dữ liệu
        if (username_found)
        {
            // Gửi thông báo yêu cầu nhập password cho client
            char response[] = "Vui lòng nhập mật khẩu: ";
            sendto(server_sock, response, strlen(response), 0,
                   (struct sockaddr *)&client_addr, client_addr_len);

            // Nhận password từ client
            char password[MAX_PASSWORD_LENGTH];
            ssize_t password_length = recvfrom(server_sock, password, sizeof(password), 0,
                                               (struct sockaddr *)&client_addr, &client_addr_len);

            if (password_length == -1)
            {
                perror("Lỗi nhận dữ liệu");
                continue;
            }

            // Kiểm tra đăng nhập
            int login_result = check_login(accounts, num_accounts, username, password);

            // Xử lý kết quả đăng nhập
            char login_response[MAX_RESPONSE_LENGTH];
            switch (login_result)
            {
            case -2:
                strcpy(login_response, "Tài khoản đã bị khóa hoặc chưa kích hoạt");
                break;
            case -1:
                strcpy(login_response, "Mật khẩu không đúng");
                break;
            case 0:
                strcpy(login_response, "Tài khoản không tồn tại");
                break;
            case 1:
                strcpy(login_response, "OK");
                break;
            }

            // Gửi kết quả đăng nhập cho client
            sendto(server_sock, login_response, strlen(login_response), 0,
                   (struct sockaddr *)&client_addr, client_addr_len);

            // Nếu đăng nhập thành công
            if (login_result == 1)
            {
                while (1)
                {
                    // Nhận yêu cầu từ client
                    char request[MAX_RESPONSE_LENGTH];
                    ssize_t request_length = recvfrom(server_sock, request, sizeof(request), 0,
                                                      (struct sockaddr *)&client_addr, &client_addr_len);

                    if (request_length == -1)
                    {
                        perror("Lỗi nhận dữ liệu");
                        continue;
                    }

                    // Kiểm tra yêu cầu đổi mật khẩu
                    if (strcmp(request, "changepassword") == 0)
                    {
                        // Gửi thông báo yêu cầu nhập mật khẩu mới cho client
                        char response[] = "Vui lòng nhập mật khẩu mới: ";
                        sendto(server_sock, response, strlen(response), 0,
                               (struct sockaddr *)&client_addr, client_addr_len);

                        // Nhận mật khẩu mới từ client
                        char new_password[MAX_PASSWORD_LENGTH];
                        ssize_t new_password_length = recvfrom(server_sock, new_password, sizeof(new_password), 0,
                                                               (struct sockaddr *)&client_addr, &client_addr_len);

                        if (new_password_length == -1)
                        {
                            perror("Lỗi nhận dữ liệu");
                            continue;
                        }

                        // Kiểm tra tính hợp lệ của mật khẩu mới và thực hiện mã hóa
                        change_password(accounts, num_accounts, username, new_password, server_sock);
                    }
                    else if (strcmp(request, "bye") == 0)
                    {
                        // Xử lý yêu cầu đăng xuất
                        char response[] = "Đăng xuất thành công";
                        sendto(server_sock, response, strlen(response), 0,
                               (struct sockaddr *)&client_addr, client_addr_len);
                        break;
                    }
                }
            }
        }
    }
    free(accounts);
    return 0;
}
