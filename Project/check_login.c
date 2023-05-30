#include "check_login.h"
#include "account.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/select.h>

extern Account accounts[MAX_ACCOUNTS];
extern int num_accounts;

void handle_login(char *buff, int sd)
{
    char username[BUFF_SIZE], password[BUFF_SIZE];
    sscanf(buff, "%s %s", username, password);
    bool username_found = false;
    for (int i = 0; i < num_accounts; i++)
    {
        if (strcmp(accounts[i].userID, username) == 0)
        {
            username_found = true;
            if (accounts[i].status == 0)
            {
                char reply[] = "This account is blocked";
                send(sd, reply, strlen(reply), 0);
                break;
            }

            if (accounts[i].client_id != -1)
            {
                char reply[] = "This account is already logged in from another client";
                send(sd, reply, strlen(reply), 0);
                break;
            }

            if (strcmp(accounts[i].password, password) == 0)
            {
                printf("Welcome %s\n", username);
                char reply[] = "Login success";
                send(sd, reply, strlen(reply), 0);
                accounts[i].client_id = sd;
                accounts[i].logged_in = 1;
                break;
            }

            else
            {
                accounts[i].attempt++;
                if (accounts[i].attempt >= 3)
                {
                    accounts[i].status = 0;
                    update_accounts();
                    char reply[] = "This account is blocked";
                    send(sd, reply, strlen(reply), 0);
                }
                else
                {
                    char reply[] = "Wrong password";
                    send(sd, reply, strlen(reply), 0);
                }
                break;
            }
        }
    }
    if (!username_found)
    {
        char reply[] = "User not found";
        send(sd, reply, strlen(reply), 0);
    }
}
