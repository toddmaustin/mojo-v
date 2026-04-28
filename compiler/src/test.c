#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SECRET __attribute__((annotate("secret")))

uint8_t foo(void) {
    uint8_t SECRET secret_byte = 0x37;
    uint8_t public_byte = 0x81;

    uint8_t a = secret_byte ^ public_byte;
    uint8_t b = public_byte + 0x10;

    if (b == 0x71) {
        return b;
    }

    return a;
}

int main(void) {
    printf("Hello, world!\n");

    uint8_t x = foo();
    return EXIT_SUCCESS;
}
