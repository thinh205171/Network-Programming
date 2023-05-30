#ifndef ACCOUNT_H
#define ACCOUNT_H

#define BUFF_SIZE 1024
#define MAX_ACCOUNTS 100

extern int num_accounts;

typedef struct
{
    char userID[BUFF_SIZE];
    char password[BUFF_SIZE];
    int status;
    int client_id;
    int logged_in;
    int attempt;
} Account;

void load_accounts();
void update_accounts();
Account *get_logged_in_account(int client_id);

#endif
