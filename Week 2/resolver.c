#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./resolver [domain or IP address]\n");
        return 1;
    }

    char *input = argv[1];
    struct hostent *host;

    // Check if input is a valid IP address
    struct in_addr ip_addr;
    int ret = inet_pton(AF_INET, input, &ip_addr);
    if (ret == 1)
    { // input is a valid IP address
        host = gethostbyaddr(&ip_addr, sizeof(ip_addr), AF_INET);
        if (host == NULL)
        {
            printf("not found information\n");
            return 1;
        }
        printf("Official name: %s\n", host->h_name);

        // Print alias names (if any)
        int i = 0;
        while (host->h_aliases[i] != NULL)
        {
            printf("Alias name: %s\n", host->h_aliases[i]);
            i++;
        }
    }
    else
    { // input is a domain name or invalid IP address
        int i;
        for (i = 0; input[i] != '\0'; i++)
        {
            if (!isdigit(input[i]) && input[i] != '.')
            {
                break;
            }
        }
        if (input[i] == '\0') // input contains only digits and dots
        {
            printf("not found information\n");
            return 1;
        }
        host = gethostbyname(input);
        if (host == NULL)
        {
            printf("not found information\n");
            return 1;
        }
        printf("Official IP: %s\n", inet_ntoa(*((struct in_addr *)host->h_addr)));

        // Print alias IP addresses (if any)
        i = 1;
        while (host->h_addr_list[i] != NULL)
        {
            struct in_addr ip_addr;
            memcpy(&ip_addr, host->h_addr_list[i], sizeof(ip_addr));
            printf("Alias IP: %s\n", inet_ntoa(ip_addr));
            i++;
        }
    }

    return 0;
}
