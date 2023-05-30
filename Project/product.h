#ifndef PRODUCT_H
#define PRODUCT_H

typedef struct {
    char name[100];
    int price;
} Product;

void readProducts(Product products[], int maxProducts, const char* filename, int* numProducts);

#endif // PRODUCT_H
