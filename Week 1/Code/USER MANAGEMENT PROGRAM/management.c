#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum statusOfAccount 
{
    active = 1,
    blocked = 0
};

typedef struct account
{
    char username[100]; 
    char password[100];
    enum statusOfAccount status;
    int signInStatus; // 1: Account has been signed in, 0: otherwise
    struct account *next;
} Account;

Account *accounts; // linked list 

int checkSignIn = 0; // 1: Exist account signed in system, 0: otherwise

Account *readFile(char *filename) // read file account.txt and add data to linked list 
{
    FILE *fp;
    Account *head = NULL;
    Account *tail = NULL;
    char line[100];
    char *token;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Can not open file %s\n", filename);
        return NULL;
    }

    while (fgets(line, 100, fp) != NULL)
    {
        Account *new_account = (Account *)malloc(sizeof(Account));

        token = strtok(line, " "); 
        strcpy(new_account->username, token); // get username

        token = strtok(NULL, " ");
        strcpy(new_account->password, token); // get password

        token = strtok(NULL, " ");
        new_account->status = atoi(token); // get status

        new_account->signInStatus = 0; // initialize sign in status of account = 0 

        new_account->next = NULL;

        if (head == NULL) // empty linked list
        {
            head = new_account;
            tail = new_account;
        }

        else // linked list has data
        {
            tail->next = new_account;
            tail = new_account;
        }
    }

    fclose(fp);

    return head;
}

void print(Account *head)
{
    Account *curr = head;
    while (curr != NULL) // traverse linked list
    {
        printf("%s %s %d\n", curr->username, curr->password, curr->status);
        curr = curr->next;
    }
}

void registerAccount() // register new account
{
    char username[50];
    char password[50];
    printf("Username: ");
    scanf("%s", username);
    Account *curr = accounts; 
    while (curr != NULL)
    {
        if (strcmp(curr->username, username) == 0) // if input username has been existed
        {
            printf("Account xisted.\n");
            return;
        }
        curr = curr->next;
    }
    printf("Password: ");
    scanf("%s", password);

    // write new account to file
    Account *newAccount = (Account *)malloc(sizeof(Account));
    strcpy(newAccount->username, username);
    strcpy(newAccount->password, password);
    newAccount->status = active;
    newAccount->signInStatus = 0;
    newAccount->next = NULL;

    if (accounts == NULL) // if linked list is empty
    {
        accounts = newAccount;
    }
    else // traverse linked list accounts, then add new account to end of accounts list
    {
        Account *curr = accounts;
        while (curr->next != NULL)
        {
            curr = curr->next;
        }
        curr->next = newAccount;
    }

    // add new account to end of file account.txt
    FILE *fp = fopen("account.txt", "a");
    if (fp == NULL) 
    {
        printf("Can not open file\n");
        return;
    }
    fprintf(fp, "%s %s %d\n", newAccount->username, newAccount->password, newAccount->status);
    fclose(fp);

    printf("Successful registration\n");
}

void signIn() // sign in system
{
    if (checkSignIn == 0) // if no account has signed in system
    {
        char username[50];
        char password[50];
        int attempts = 0;

        printf("Username: ");
        scanf("%s", username);

        Account *curr = accounts;

        // traverse accounts list
        while (curr != NULL)
        {
            if (strcmp(curr->username, username) == 0) // if input username has been found
            {
                do
                {
                    printf("Password: ");
                    scanf("%s", password);
                    if (strcmp(curr->password, password) == 0) // if input password has been found
                    {
                        if (curr->status == blocked) // if status of account is block 
                            printf("Account is blocked.\n");
                        else // if status of account is active
                        {
                            printf("Hello %s\n", username);
                            checkSignIn = 1; // system has account signed in s
                            curr->signInStatus = 1;
                        }
                        return;
                    }
                    else // if input password has not been found
                    {
                        attempts++;
                        printf("Password is incorrect. ");
                        if (attempts == 4) // if input password reaches 4 times incorrect attemp
                        {
                            printf("Account is blocked.");
                            curr->status = blocked; // change status of account to block

                            FILE *fp = fopen("account.txt", "w");
                            if (fp == NULL)
                            {
                                printf("Can not open file\n");
                                return;
                            }
                            // update new status of account
                            Account *acc = accounts;
                            while (acc != NULL)
                            {
                                fprintf(fp, "%s %s %d\n", acc->username, acc->password, acc->status);
                                acc = acc->next;
                            }

                            fclose(fp);
                            return;
                        }
                        printf("\nPlease enter password again!!.\n");
                    }
                } while (attempts <= 4);
            }
            curr = curr->next;
        }
        printf("Cannot find account.\n");
    }
    else // if exist account sign in systems
    {
        printf("Need to sign out first.\n");
    }
}

void search() // search account in system
{
    char username[50];
    printf("Username: ");
    scanf("%s", username);
    Account *curr = accounts;
    while (curr != NULL)
    {
        if (strcmp(curr->username, username) == 0) // if input username has been found
        {
            if (curr->status == active) // if status of account is active
                printf("Account is active.");
            else if (curr->status == blocked) // if status of account is blocked
                printf("Account is blocked.");
            return;
        }
        curr = curr->next;
    }
    printf("Cannot find account."); // can not find account is system
    return;
}

void signOut() // sign out system
{
    char username[50];
    printf("Username: ");
    scanf("%s", username);
    Account *curr = accounts;
    while (curr != NULL)
    {
        if (strcmp(curr->username, username) == 0) // if input username has been found
        {
            if (curr->signInStatus == 1)
            {
                printf("Goodbye %s.", username);
                checkSignIn = 0; // system has no account signed in
            }
            else
                printf("Account is not sign in.");
            return;
        }
        curr = curr->next;
    }
    printf("Cannot find account."); // if input account has not been found
    return;
}

int main()
{
    accounts = readFile("account.txt"); 
    int choice;

    do
    {
        printf("\nUSER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Sign in\n");
        printf("3. Search\n");
        printf("4. Sign out\n");
        printf("Your choice (1-4, other to quit): ");

        if (scanf("%d", &choice) != 1) // if input is not number
        {
            printf("Invalid input.\n");
            while (getchar() != '\n')
                ;
            continue;
        }

        switch (choice)
        {
        case 1:
            registerAccount();
            break;
        case 2:
            signIn();
            break;
        case 3:
            search();
            break;
        case 4:
            signOut();
            break;
        default:
            printf("Goodbye!\n");
            exit(0);
        }
    } while (1);
    return 0;
}
