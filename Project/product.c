#include "product.h"
#include <stdio.h>
#include <string.h>

void readProducts(Product products[], int maxProducts, const char* filename, int* numProducts)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Cannot open %s\n", filename);
        return;
    }
    
    *numProducts = 0;
    int count = 0;
    char line[100];
    
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Xóa ký tự newline
        line[strcspn(line, "\n")] = '\0';
        
        // Đọc tên sản phẩm
        strcpy(products[count].name, line);
        
        // Đọc giá tiền
        if (fgets(line, sizeof(line), file) != NULL)
        {
            sscanf(line, "%d", &products[count].price);
            count++;
        }
        
        // Kiểm tra xem đã đạt đủ số sản phẩm tối đa chưa
        if (count == maxProducts)
            break;
    }
    
    *numProducts = count;
    
    fclose(file);
}
