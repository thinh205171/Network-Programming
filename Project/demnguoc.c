#include "demnguoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void play_game(int product_price)
{
    // Sử dụng hàm random để tạo giá trị sai ngẫu nhiên
    srand(time(NULL));
    // int target_price = rand() % 100000000;
    int target_price = 100000;

    // In giá trị sai lên màn hình cho người chơi nhìn thấy
    printf("Giá sai: %d\n", target_price);

    // Thiết lập thời gian đếm ngược là 60 giây
    int countdown = 60, higher = product_price > target_price, lower = product_price < target_price;

    printf("Sản phẩm của chúng tôi cao hơn hay thấp hơn so với giá trên: ");

    // Nhận input từ người chơi và kiểm tra đúng/sai
    while (countdown > 0)
    {
        char input[100];
        fgets(input, sizeof(input), stdin);
        int guess = atoi(input);

        if (guess > target_price && lower)
        {
            printf("Giá sản phẩm thấp hơn\n");
        }
        if (guess < target_price && higher)
        {
            printf("Giá sản phẩm cao hơn\n");
        }
        sleep(1);
        countdown--;
    }
    printf("Hết thời gian! Bạn đã thua cuộc.\n");
}