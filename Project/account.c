#include "account.h"
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

Account accounts[MAX_ACCOUNTS];
int num_accounts = 0;

void load_accounts()
{
    FILE *file = fopen("Account.txt", "r");
    if (file == NULL)
    {
        printf("Cannot open account.txt\n");
        return;
    }

    while (!feof(file) && num_accounts < MAX_ACCOUNTS)
    {
        fscanf(file, "%s %s %d %d\n", accounts[num_accounts].userID, accounts[num_accounts].password, &accounts[num_accounts].point, &accounts[num_accounts].status);
        accounts[num_accounts].client_id = -1;
        accounts[num_accounts].attempt = 0;
        num_accounts++;
    }

    fclose(file);
}

void update_accounts()
{
    FILE *file = fopen("Account.txt", "w");
    if (file == NULL)
    {
        printf("Cannot open account.txt\n");
        return;
    }

    for (int i = 0; i < num_accounts; i++)
    {
        fprintf(file, "%s %s %d %d\n", accounts[i].userID, accounts[i].password, accounts[i].point, accounts[i].status);
    }

    fclose(file);
}

Account *get_logged_in_account(int client_id)
{
    for (int i = 0; i < num_accounts; i++)
    {
        if (accounts[i].client_id == client_id)
        {
            return &accounts[i];
        }
    }
    return NULL;
}
