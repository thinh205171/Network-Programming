#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>

#define BUFF_SIZE 1024
#define BACKLOG 2

void process_string(char *input, char *letters, char *digits);
void receive_file(int conn_sock);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./server PortNumber\n");
        return 0;
    }

    int PORT = atoi(argv[1]);
    int listen_sock, conn_sock; /* file descriptors */
    char recv_data[BUFF_SIZE], command[BUFF_SIZE];
    int bytes_sent, bytes_received, command_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t sin_size;

    // Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }

    // Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);              /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    // Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    // Bổ sung mã xử lý dữ liệu từ client
    while (1)
    {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        // start conversation
        while (1)
        {
            command_received = recv(conn_sock, command, BUFF_SIZE - 1, 0); // blocking
            if (command_received <= 0)
            {
                printf("\nConnection closed");
                break;
            }
            command[command_received] = '\0';

            if (strcmp(command, "string") == 0)
            {
                // receives message from client
                bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); // blocking
                if (bytes_received <= 0)
                {
                    printf("\nConnection closed");
                    // break;
                }
                else
                {
                    recv_data[bytes_received] = '\0';
                    printf("Receive: %s \n\n", recv_data);
                }

                if (strcmp(recv_data, "exit") == 0)
                {
                    printf("\nConnection closed. Client sent an empty string. Server exits.\n");
                    return 0;
                }

                char letters[BUFF_SIZE] = {0};
                char digits[BUFF_SIZE] = {0};
                process_string(recv_data, letters, digits);

                if (strcmp(letters, "Error! Invalid input") != 0)
                {
                    if (strlen(letters) == 0)
                        send(conn_sock, "empty", strlen("empty"), 0);
                    else
                    {
                        // echo letter to client
                        bytes_sent = send(conn_sock, letters, strlen(letters), 0); /* send to the client welcome message */
                        if (bytes_sent <= 0)
                        {
                            printf("\nConnection closed");
                            break;
                        }
                    }

                    char delimiter = '\n';
                    bytes_sent = send(conn_sock, &delimiter, 1, 0);
                    if (bytes_sent <= 0)
                    {
                        printf("\nConnection closed");
                        break;
                    }

                    if (strlen(digits) == 0)
                        send(conn_sock, "empty", strlen("empty"), 0);
                    else
                    {
                        // echo digit to client
                        bytes_sent = send(conn_sock, digits, strlen(digits), 0); /* send to the client welcome message */
                        if (bytes_sent <= 0)
                        {
                            printf("\nConnection closed");
                            break;
                        }
                    }
                }
                else
                {
                    bytes_sent = send(conn_sock, letters, strlen(letters), 0); /* send to the client welcome message */
                    if (bytes_sent <= 0)
                    {
                        printf("\nConnection closed");
                        break;
                    }
                }
            }

            if (strcmp(command, "file") == 0)
            {
                // printf("Send file\n");
                receive_file(conn_sock);
                // break;
            }
        } // end conversation
        close(conn_sock);
    }
    close(listen_sock);
    return 0;
}

void process_string(char *input, char *letters, char *digits)
{
    for (int i = 0, j = 0, k = 0; input[i]; i++)
    {
        if (isalpha(input[i]))
        {
            letters[j++] = input[i];
        }
        else if (isdigit(input[i]))
        {
            digits[k++] = input[i];
        }
        else
        {
            // Ký tự không phải là chữ cái hoặc chữ số
            strcpy(letters, "Error! Invalid input");
            break;
        }
    }
}

void receive_file(int conn_sock)
{
    char file_name[BUFF_SIZE];
    int file_name_len = recv(conn_sock, file_name, BUFF_SIZE - 1, 0);
    if (strcmp(file_name, "error") == 0)
    {
        printf("Không nhận được tên file từ client.\n\n");
        return;
    }

    file_name[file_name_len] = '\0';

    printf("Đã nhận file %s thành công.\n", file_name);

    FILE *file = fopen(file_name, "wb");
    if (file == NULL)
    {
        printf("Không thể mở file để ghi dữ liệu.\n");
        return;
    }

    // Nhận kích thước file
    long file_size;
    recv(conn_sock, &file_size, sizeof(file_size), 0);

    // Nhận dữ liệu file
    char buffer[BUFF_SIZE];
    int bytes_received;
    long bytes_total = 0;
    while (bytes_total < file_size && (bytes_received = recv(conn_sock, buffer, BUFF_SIZE, 0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file);
        bytes_total += bytes_received;
    }

    fclose(file);

    // Mở lại file để đọc nội dung
    file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Không thể mở file để đọc dữ liệu.\n");
        return;
    }

    printf("Nội dung file:\n");

    char content[BUFF_SIZE];
    while (fgets(content, BUFF_SIZE, file) != NULL)
    {
        printf("%s", content);
    }

    printf("\n\n");

    fclose(file);

    // Gửi xác nhận cho client
    char success_msg[] = "File received successfully";
    send(conn_sock, success_msg, strlen(success_msg), 0);
}
