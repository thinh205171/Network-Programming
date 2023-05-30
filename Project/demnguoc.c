#include "demnguoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void play_game()
{
    // Sử dụng hàm random để tạo giá trị sai ngẫu nhiên
    srand(time(NULL));
    int target_price = rand() % 100000000;

    // In giá trị sai lên màn hình cho người chơi nhìn thấy
    printf("Giá sai: %d\n", target_price);

    // Thiết lập thời gian đếm ngược là 60 giây
    int countdown = 60;

    // Nhận input từ người chơi và kiểm tra đúng/sai
    while (countdown > 0)
    {
        printf("Đặt những miếng gỗ xanh lên vị trí trên hay dưới giá sai: ");
        char input[100];
        fgets(input, sizeof(input), stdin);
        int guess = atoi(input);

        if (guess == target_price)
        {
            printf("Chính xác! Bạn đã thắng!\n");
            return;
        }
        else if (guess > target_price)
        {
            printf("Giá sai thấp hơn\n");
        }
        else
        {
            printf("Giá sai cao hơn\n");
        }
        sleep(1);
        countdown--;
    }
    printf("Hết thời gian! Bạn đã thua cuộc.\n");
}